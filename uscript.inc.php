<?php

class CodeGen {
    public $blacklist_re = '@(editor_(init|deinit|run|debug_dump)|cre|listener)@';
    public $valid_pointer_re = '@(bline_t|buffer_t|bview_t|cursor_t|editor_t|mark_t|observer_t|void|char|size_t|int)@';

    function run() {
        $protos = $this->getProtoMap();
        $hardcoded = $this->getHardcodedProtoMap();
        $protos = array_merge($protos, $hardcoded);
        usort($protos, function ($a, $b) {
            return strcmp($a->name, $b->name);
        });
        $this->printFuncs($protos);
        $this->printLibTable($protos);
    }

    function getProtoMap() {
        $mlbuf_h = __DIR__ . '/mlbuf.h';
        $mle_h = __DIR__ . '/mle.h';
        $grep_re = '^\S+ \*?(editor|bview|buffer|cursor|mark)_.*\);$';
        $grep_cmd = sprintf(
            'grep -hP %s %s %s',
            escapeshellarg($grep_re),
            escapeshellarg($mlbuf_h),
            escapeshellarg($mle_h)
        );
        $proto_strs = explode("\n", shell_exec($grep_cmd));
        $proto_strs = array_filter($proto_strs, function($proto_str) {
            if (strlen(trim($proto_str)) <= 0) {
                return false;
            }
            if (preg_match($this->blacklist_re, $proto_str)) {
                return false;
            }
            return true;
        });
        $protos = array_map(function($proto_str) {
            return new Proto($proto_str);
        }, $proto_strs);
        return array_combine(
            array_column($protos, 'name'),
            $protos
        );
    }

    function getHardcodedProtoMap() {
        $uscript_c = __DIR__ . '/uscript.c';
        $grep_re = '^// foreign static (?<ret_type>\S+) _uscript_func_(?<name>[^\(]+)\((?<params>[^\)]*)\)$';
        $grep_cmd = sprintf(
            'grep -hP %s %s',
            escapeshellarg($grep_re),
            escapeshellarg($uscript_c)
        );
        $hardcoded_strs = explode("\n", shell_exec($grep_cmd));
        $hardcoded_strs = array_filter($hardcoded_strs);
        $protos = array_map(function($hardcoded_str) use ($grep_re) {
            $m = null;
            if (!preg_match("@{$grep_re}@", $hardcoded_str, $m)) {
                throw new RuntimeException("Failed to parse hardcoded proto: $hardcoded_str");
            }
            $params = preg_split('@\s*,\s*@', $m['params']);
            $params = array_map(function($param) {
                return sprintf("void *%s", $param);
            }, $params);
            $params_str = implode(', ', $params);
            $proto_str = sprintf("void %s(%s);", $m['name'], $params_str);
            $proto = new Proto($proto_str);
            $proto->is_hardcoded = true;
            return $proto;
        }, $hardcoded_strs);
        return array_combine(
            array_column($protos, 'name'),
            $protos
        );
    }

    function printLibTable($protos) {
        echo 'static const struct luaL_Reg mle_lib[] = {' . "\n";
        foreach ($protos as $proto) {
            printf('    { "%s", %s },' . "\n", $proto->name, $proto->c_func);
        }
        echo "    { NULL, NULL }\n";
        echo "};\n\n";
    }

    function printFuncs($protos) {
        foreach ($protos as $proto) {
            $this->printFunc($proto);
        }
    }

    function printFunc($proto) {
        $is_hardcoded = $proto->is_hardcoded;
        printf(
            "%sstatic int %s(lua_State *L) {\n",
            $is_hardcoded ? '// ' : '',
            $proto->c_func
        );
        if (!$is_hardcoded) {
            printf("    %s%srv;\n", $proto->ret_type, $proto->ret_is_pointer ? '' : ' ');
            foreach ($proto->params as $param) {
                if ($param->is_ret) {
                    printf("    %s%s%s = %s;\n", $param->ret_type, $param->ret_is_pointer ? '' : ' ', $param->name, $param->ret_zero_val);
                } else {
                    printf("    %s%s%s;\n", $param->type, $param->is_pointer ? '' : ' ', $param->name);
                }
            }
            $param_num = 1;
            foreach ($proto->params as $param) {
                if ($param->is_ret) continue;
                $this->fromLua($param, $param_num++);
            }
            $call_names = array_map(function($param) {
                return $param->call_name;
            }, $proto->params);
            printf("    rv = %s(%s);\n", $proto->name, implode(', ', $call_names));
            printf("    lua_createtable(L, 0, %d);\n", $proto->ret_count);
            $this->toLua('rv', $proto->ret_type);
            foreach ($proto->params as $param) {
                if (!$param->is_ret) continue;
                $this->toLua($param->name, $param->type);
            }
            printf("    lua_pushvalue(L, -1);\n");
            printf("    return 1;\n");
        }
        printf("%s}\n\n", $is_hardcoded ? '// ' : '');
    }

    function toLua($name, $type) {
        printf('    lua_pushstring(L, "%s");' . "\n", $name);
        if (strpos($type, 'int') !== false || $type === 'char') {
            printf("    lua_pushinteger(L, (lua_Integer)%s);\n", $name);
        } else if ($type === 'char *' || $type === 'const char *') {
            printf("    lua_pushstring(L, (const char*)%s);\n", $name);
        } else if (preg_match($this->valid_pointer_re, $type)) {
            printf("    lua_pushpointer(L, (void*)%s);\n", $name);
        } else {
            throw new RuntimeException("Unhandled type: $type");
        }
        printf("    lua_settable(L, -3);\n");
    }

    function fromLua($param, $slot) {
        $type = $param->type;
        $name = $param->name;
        if (strpos($type, 'int') !== false || $type === 'char' || $type === 'size_t') {
            if ($param->is_opt) {
                printf("    %s = (%s)luaL_optinteger(L, %d, 0);\n", $name, $type, $slot);
            } else {
                printf("    %s = (%s)luaL_checkinteger(L, %d);\n", $name, $type, $slot);
            }
        } else if ($type === 'float' || $type === 'double') {
            if ($param->is_opt) {
                printf("    %s = (%s)luaL_optnumber(L, %d, 0);\n", $name, $type, $slot);
            } else {
                printf("    %s = (%s)luaL_checknumber(L, %d);\n", $name, $type, $slot);
            }
        } else if ($type === 'char *' || $type === 'const char *') {
            if ($param->is_opt) {
                printf("    %s = (%s)luaL_optstring(L, %d, NULL);\n", $name, $type, $slot);
            } else {
                printf("    %s = (%s)luaL_checkstring(L, %d);\n", $name, $type, $slot);
            }
        } else if (preg_match($this->valid_pointer_re, $type)) {
            if ($param->is_opt) {
                printf("    %s = (%s)luaL_optpointer(L, %d, NULL);\n", $name, $type, $slot);
            } else {
                printf("    %s = (%s)luaL_checkpointer(L, %d);\n", $name, $type, $slot);
            }
        } else {
            throw new RuntimeException("Unhandled type: $type");
        }
    }
}

class Proto {
    function __construct($proto_str) {
        $parse_re = '@^(?<ret_type>\S+) (?<ret_type_ptr>\*?)(?<name>[^\(]+)\((?<params>.*?)\);$@';
        $match = [];
        if (!preg_match($parse_re, $proto_str, $match)) {
            throw new RuntimeException("Could not parse proto: $proto_str");
        }
        $this->name = $match['name'];
        $this->c_func = '_uscript_func_' . $this->name;
        $this->ret_type = trim($match['ret_type'] . ' ' . $match['ret_type_ptr']);
        $this->ret_is_pointer = substr($this->ret_type, -1) === '*';
        $this->params = [];
        $this->ret_count = 1;
        $this->has_funcs = false;
        $this->is_hardcoded = false;
        if (empty($match['params'])) {
            return;
        }
        $param_strs = preg_split('@\s*,\s*@', $match['params']);
        foreach ($param_strs as $param_str) {
            $param = new Param($param_str);
            $this->params[] = $param;
            if ($param->is_ret) {
                $this->ret_count += 1;
            }
            if ($param->is_func) {
                $this->has_funcs = true;
            }
        }
    }
}

class Param {
    function __construct($param_str) {
        $parts = preg_split('@\s+@', $param_str);
        $name_w_ptr = current(array_slice($parts, -1));
        $type_wout_ptr = implode(' ', array_slice($parts, 0, -1));
        $ptr_i = 0;
        while (substr($name_w_ptr, $ptr_i, 1) === '*') $ptr_i += 1;
        $ptr = substr($name_w_ptr, 0, $ptr_i);
        $this->type = trim($type_wout_ptr . ' ' . $ptr);
        $this->name = substr($name_w_ptr, $ptr_i);
        $this->is_func = preg_match('@^fn_@', $this->name);
        $this->is_opt = preg_match('@^opt(ret)?_@', $this->name);
        $this->is_ret = preg_match('@^(opt)?ret_@', $this->name);
        $this->is_optret = preg_match('@^optret_@', $this->name);
        $this->is_pointer = strpos($this->type, '*') !== false;
        $this->is_string = $this->type === 'char *';
        if ($this->is_ret) {
            if (!$this->is_pointer) {
                throw new RuntimeException("Expected ret param ptr: $param_str");
            }
            $this->ret_type = trim(substr($this->type, 0, -1));
            $this->ret_is_pointer = strpos($this->ret_type, '*') !== false;
            $this->ret_zero_val = $this->ret_is_pointer ? 'NULL' : '0';
        } else {
            $this->ret_type = null;
            $this->ret_is_pointer = false;
            $this->ret_zero_val = '0';
        }
        $this->call_name = ($this->is_ret ? '&' : '') . $this->name;
    }
}

(new CodeGen())->run();
