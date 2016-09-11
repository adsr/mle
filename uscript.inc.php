<?php

class UscriptCodeGen {
    const PROTO_GREP_CMD = "grep -hP '^int (mark)' %s | grep -Pv '(callback|pcre)'";
    const PROTO_PARSE_RE = '@^int (?<name>[^\(]+)\((?<params>.*?)\);$@';

    private $max_ret_var = 0;
    private $type_count_map = [];

    function run() {
        $protos = $this->getProtos();
        $call_code = $this->generateCallCode($protos);
        $def_code = $this->generateVarDefs();
        $func_code = $def_code . $call_code;
        $func_code = trim(str_replace("\n", "\n    ", $func_code));
        echo '    ' . $func_code . "\n";
    }

    function getProtos() {
        $proto_strs = $this->getProtoStrings();
        return $this->parseProtoStrings($proto_strs);
    }

    function getProtoStrings() {
        $mlbuf_h = __DIR__ . '/mlbuf/mlbuf.h';
        $mle_h = __DIR__ . '/mle.h';
        $grep_cmd = sprintf(
            self::PROTO_GREP_CMD,
            escapeshellarg($mlbuf_h) . ' ' . escapeshellarg($mle_h)
        );
        $proto_strs = shell_exec($grep_cmd);
        return array_filter(explode("\n", $proto_strs));
    }

    function parseProtoStrings($proto_strs) {
        $protos = [];
        foreach ($proto_strs as $proto_str) {
            $m = null;
            if (!preg_match(self::PROTO_PARSE_RE, $proto_str, $m)) {
                throw new RuntimeException("Could not parse proto: <{$proto_str}>");
            }
            $params = [];
            $param_strs = preg_split('@\s*,\s*@', $m['params']);
            $num_rets = 0;
            $num_params = 0;
            foreach ($param_strs as $param_str) {
                list($param_type, $param_name) = preg_split('@\s+@', $param_str, 2);
                $param_is_ret = substr($param_name, 0, 4) === 'ret_'
                    || substr($param_name, 0, 7) === 'optret_';
                if ($param_is_ret) {
                    $param_type = substr($param_type, 0, -1);
                    $num_rets += 1;
                } else {
                    $num_params += 1;
                }
                $params[$param_name] = [
                    'ret_name' => preg_replace('@^(opt)?ret_@', '', $param_name),
                    'is_ret' => $param_is_ret,
                    'type' => $param_type,
                ];
            }
            $protos[$m['name']] = [
                'name' => $m['name'],
                'params' => $params,
                'num_rets' => $num_rets,
                'num_params' => $num_params,
            ];
        }
        return $protos;
    }

    function generateCallCode($protos) {
        $code = '';
        $all_type_count = [];
        $max_num_ret = 0;
        $code = "do { if (0) {\n";
        foreach ($protos as $func_name => $proto) {
            $code .= sprintf('} else if (strcmp(msg->cmd_name, "%s") == 0) {', $func_name) . "\n";
            $code .= sprintf('    if (msg->params_len != %d) break;', $proto['num_params']) . "\n";
            $code .= $this->generateCallCodeForProto($proto);
        }
        $code .= "} } while(0);\n";
        $code .= "rf = _uscript_write_response(uscript, msg, rc, retvar, retvar_name, retvar_is_num, retvar_count);\n";
        $code .= "for (i = 0; i < retvar_count; i++) free(retvar[i]);\n";
        $code .= "return rf;\n";
        return $code;
    }

    function generateCallCodeForProto($proto) {
        $param_num = 0;
        $ret_num = 0;
        $call_args = [];
        $type_count = [];
        $code = '';
        $ret_code = '';
        foreach ($proto['params'] as $param_name => $param) {
            $type = $param['type'];
            $is_ret = $param['is_ret'];
            $arg_var = $this->getTmpVar($proto, $type);
            if (!$is_ret) {
                // Convert msg->params string to real type
                if ($type == 'char*') {
                    $fmt = '%s = msg->params[%d];';
                } else if (strpos($type, '*') !== false) {
                    $fmt = "%s = MLE_USCRIPT_STR_TO_PTR({$type}, msg->params[%d]);";
                } else if (strpos($type, 'uint') !== false || $type == 'size_t') {
                    $fmt = "%s = ($type)strtoull(msg->params[%d], NULL, 10);";
                } else if (strpos($type, 'int') !== false) {
                    $fmt = "%s = ($type)strtoll(msg->params[%d], NULL, 10);";
                } else {
                    throw new RuntimeException("Unrecognized param type: {$type}");
                }
                $code .= '    ' . sprintf($fmt, $arg_var, $param_num) . "\n";
            } else {
                // Convert retvars to string
                $ret_is_num = false;
                if ($type == 'char*') {
                    $fmt = '%s = strdup(%s);';
                } else if (strpos($type, '*') !== false) {
                    $fmt = 'MLE_USCRIPT_PTR_TO_STR(ptrbuf, %2$s); %1$s = strdup(ptrbuf);';
                } else if (strpos($type, 'uint') !== false || $type == 'size_t') {
                    $fmt = 'asprintf(&%s, "%%llu", (unsigned long long)%s);';
                    $ret_is_num = true;
                } else if (strpos($type, 'int') !== false) {
                    $fmt = 'asprintf(&%s, "%%lld", (long long)%s);';
                    $ret_is_num = true;
                } else {
                    throw new RuntimeException("Unrecognized ret type: {$type}");
                }
                $ret_code .= '    ' . sprintf('retvar_name[%d] = "%s";', $ret_num, $param['ret_name']) . "\n";
                $ret_code .= '    ' . sprintf('retvar_is_num[%d] = %d;', $ret_num, $ret_is_num ? 1 : 0) . "\n";
                $ret_code .= '    ' . sprintf($fmt, $this->getRetVar($proto, $ret_num), $arg_var) . "\n";
                $ret_num += 1;
            }
            $call_args[] = ($is_ret ? '&' : '') . $arg_var;
            $param_num += 1;
        }
        $code .= sprintf('    rc = %s(%s);', $proto['name'], implode(', ', $call_args)) . "\n";
        $code .= $ret_code;
        $code .= sprintf('    retvar_count = %d;', $ret_num) . "\n";
        return $code;
    }

    function getMergedTypeCounts() {
        $all_type_count_map = [];
        foreach ($this->type_count_map as $func_name => $type_count_map) {
            foreach ($type_count_map as $type => $count) {
                if (!isset($all_type_count_map[$type]) || $all_type_count_map[$type] < $count) {
                    $all_type_count_map[$type] = $count;
                }
            }
        }
        return $all_type_count_map;
    }

    function generateVarDefs() {
        $code = '';
        $type_counts = $this->getMergedTypeCounts();
        foreach ($type_counts as $type => $count) {
            for ($i = 0; $i < $count; $i++) {
                $code .= sprintf(
                    "%s %s = %s;",
                    $type,
                    $this->getTmpVarForNum($type, $i),
                    strpos($type, '*') ? 'NULL' : '0'
                ) . "\n";
            }
        }
        $code .= "int i = 0;\n";
        $code .= "char ptrbuf[32];\n";
        $code .= "int rc = MLE_ERR;\n";
        $code .= "int rf = MLE_ERR;\n";
        $code .= "int retvar_count = 0;\n";
        $code .= sprintf('int retvar_is_num[%d];', $this->max_ret_var) . "\n";
        $code .= sprintf('char* retvar_name[%d];', $this->max_ret_var) . "\n";
        $code .= sprintf('char* retvar[%d];', $this->max_ret_var) . "\n";
        return $code;
    }

    function getTmpVar($proto, $type) {
        if (!isset($this->type_count_map[$proto['name']])) {
            $this->type_count_map[$proto['name']] = [];
        }
        $type_count_map = &$this->type_count_map[$proto['name']];
        if (!isset($type_count_map[$type])) {
            $type_count_map[$type] = 0;
        }
        $var = $this->getTmpVarForNum($type, $type_count_map[$type]);
        $type_count_map[$type] += 1;
        return $var;
    }

    function getTmpVarForNum($type, $i) {
        $type = preg_replace('@_t\b@', '', $type);
        $type = str_replace('*', 'p', $type);
        $type = preg_replace('@[^a-z_]@i', '', $type);
        return sprintf("tmp_%s_%d", $type, $i);
    }

    function getRetVar($proto, $i) {
        if ($i + 1 > $this->max_ret_var) {
            $this->max_ret_var = $i + 1;
        }
        return sprintf('retvar[%d]', $i);
    }

}

(new UscriptCodeGen())->run();
