<?php

class CodeGen {
    public $hardcode_re = '@(editor_(menu|register_cmd|get_input|prompt)|kmap|fn_)@';
    public $blacklist_re = '@(editor_(init|deinit|run|debug_dump)|cre|listener)@';

    function run() {
        $protos = $this->getProtos();
        usort($protos, function ($a, $b) {
            return strcmp($a->name, $b->name);
        });
        $this->printForeignClass($protos);
        $this->printFuncs($protos);
        $this->printBindCallback($protos);
    }

    function getProtos() {
        $mlbuf_h = __DIR__ . '/mlbuf/mlbuf.h';
        $mle_h = __DIR__ . '/mle.h';
        $grep_re = '^\S+ (editor|bview|cursor|mark)_.*\);$';
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
        return array_map(function($proto_str) {
            return new Proto($proto_str);
        }, $proto_strs);
    }

    function printForeignClass($protos) {
        echo 'const char* mle_wren = "class mle {\n\\' . "\n";
        foreach ($protos as $proto) {
            $non_ret_params = array_filter($proto->params, function($param) {
                return !$param->is_ret;
            });
            printf(
                "    foreign static %s(%s) \\n\\\n",
                $proto->name,
                implode(', ', array_column($non_ret_params, 'name'))
            );
        }
        echo '} \\' . "\n";
        echo '";' . "\n\n";
    }

    function printBindCallback($protos) {
        echo "static WrenForeignMethodFn _uscript_bind_method(WrenVM* vm, const char* module, const char* class, bool is_static, const char* sig) {\n";
        echo '    if (!is_static' . "\n";
        echo '        || strcmp(module, "main") != 0' . "\n";
        echo '        || strcmp(class, "mle") != 0' . "\n";
        echo "    ) {\n";
        echo "        return NULL;\n";
        echo "    }\n";
        echo "    if (0) {\n";
        foreach ($protos as $proto) {
            printf("    } else if (strncmp(sig, \"%s(\", %d) == 0) {\n", $proto->name, strlen($proto->name)+1);
            printf("        return _uscript_%s;\n", $proto->name);
        }
        echo "    };\n";
        echo "    return NULL;\n";
        echo "}\n\n";
    }

    function printFuncs($protos) {
        foreach ($protos as $proto) {
            $this->printFunc($proto);
        }
    }

    function printFunc($proto) {
        $is_hardcoded = preg_match($this->hardcode_re, $proto->name);
        printf(
            "%sstatic void _uscript_%s(WrenVM* vm) {\n",
            $is_hardcoded ? '// ' : '',
            $proto->name
        );
        if (!$is_hardcoded) {
            printf("    %s rv;\n", $proto->ret_type);
            foreach ($proto->params as $param) {
                if ($param->is_ret) {
                    printf("    %s %s = %s;\n", $param->ret_type, $param->name, $param->zero_val);
                } else {
                    printf("    %s %s;\n", $param->type, $param->name);
                }
            }
            $param_num = 1;
            foreach ($proto->params as $param) {
                if ($param->is_ret) continue;
                $this->fromWren($param, $param_num++);
            }
            $call_names = array_map(function($param) {
                return $param->call_name;
            }, $proto->params);
            printf("    rv = %s(%s);\n", $proto->name, implode(', ', $call_names));
            printf("    wrenEnsureSlots(vm, %d);\n", $proto->ret_count + 1);
            printf("    wrenSetSlotNewList(vm, 0);\n");
            $slot = 1;
            $this->toWren('rv', $proto->ret_type, $slot++, 0);
            foreach ($proto->params as $param) {
                if (!$param->is_ret) continue;
                $this->toWren($param->name, $param->type, $slot, 0);
                $slot += 1;
            }
        }
        printf("%s}\n\n", $is_hardcoded ? '// ' : '');
    }

    function toWren($name, $type, $slot, $list_slot) {
        if (strpos($type, 'int') !== false || $type === 'char') {
            printf("    wrenSetSlotDouble(vm, %d, (double)%s);\n", $slot, $name);
        } else if ($type === 'char*' || $type === 'const char*') {
            printf("    wrenSetSlotString(vm, %d, (const char*)%s);\n", $slot, $name);
        } else if (strpos($type, '*') !== false) {
            printf("    wrenSetSlotPointer(vm, %d, (void*)%s);\n", $slot, $name);
        } else {
            throw new RuntimeException("Unhandled type: $type");
        }
        printf("    wrenInsertInList(vm, %d, -1, %d);\n", $list_slot, $slot);
    }

    function fromWren($param, $slot) {
        $type = $param->type;
        $name = $param->name;
        if (strpos($type, 'int') !== false || $type === 'char' || $type === 'float') {
            printf("    %s = (%s)wrenGetSlotDouble(vm, %d);\n", $name, $type, $slot);
        } else if ($type === 'char*' || $type === 'const char*') {
            printf("    %s = (%s)wrenGetSlotString(vm, %d);\n", $name, $type, $slot);
        } else if (strpos($type, '*') !== false) {
            printf("    %s = (%s)wrenGetSlotPointer(vm, %d);\n", $name, $type, $slot);
        } else {
            throw new RuntimeException("Unhandled type: $type");
        }
    }
}

class Proto {
    function __construct($proto_str) {
        $parse_re = '@^(?<ret_type>\S+) (?<name>[^\(]+)\((?<params>.*?)\);$@';
        $match = [];
        if (!preg_match($parse_re, $proto_str, $match)) {
            throw new RuntimeException("Could not parse proto: $proto_str");
        }
        $this->name = $match['name'];
        $this->ret_type = $match['ret_type'];
        $this->params = [];
        $this->ret_count = 1;
        $this->has_funcs = false;
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
        $this->name = current(array_slice($parts, -1));
        $this->type = implode(' ', array_slice($parts, 0, -1));
        $this->is_func = preg_match('@^fn_@', $this->name);
        $this->is_opt = preg_match('@^opt(ret)?_@', $this->name);
        $this->is_ret = preg_match('@^(opt)?ret_@', $this->name);
        $this->is_optret = preg_match('@^optret_@', $this->name);
        $this->is_pointer = strpos($this->type, '*') !== false;
        $this->is_string = $this->type === 'char*';
        if ($this->is_ret) {
            if (substr($this->type, -1) !== '*') {
                throw new RuntimeException("Expected ret param ptr: $param_str");
            }
            $this->ret_type = substr($this->type, 0, -1);
        } else {
            $this->ret_type = null;
        }
        $this->call_name = ($this->is_ret ? '&' : '') . $this->name;
        $this->zero_val = strpos($this->ret_type, '*') !== false ? 'NULL' : '0';
    }
}

(new CodeGen())->run();
