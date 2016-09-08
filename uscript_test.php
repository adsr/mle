#!/usr/bin/env php
<?php

define('MLE_OK', 0);
define('MLE_ERR', -1);

abstract class MleUscript {
    abstract function getCommands();
    abstract function handleRequest($req);

    function run() {
        $this->registerCommands();
        try {
            $this->handleRequests();
        } catch (EOFException $e) {
        }
    }

    function registerCommands() {
        foreach ($this->getCommands() as $cmd_name) {
            $this->editor_register_cmd($cmd_name);
        }
    }

    function handleRequests() {
        while (true) {
            $req = $this->readRequest();
            $rc = $this->handleRequest($req);
            $this->send([
                'result' => [ 'rc' => $rc ],
                'error' => $rc !== 0 ? $rc : '',
                'id' => $req['id'],
            ]);
        }
    }

    function __call($method, $args) {
        $id = $this->send([
            'method' => $method,
            'params' => $args
        ]);
        $res = $this->readResponse();
        if (empty($res['id']) || $res['id'] !== $id) {
            throw new RuntimeException('Mismatched id');
        }
        return $res;
    }

    function readMessage() {
        $line = fgets(STDIN);
        if ($line === false) {
            throw new EOFException();
        }
        $line = rtrim($line);
        $res = [];
        parse_str($line, $res);
        if (!isset($res['id']) || strlen($res['id']) < 1) {
            throw new RuntimeException('Bad editor msg; no id');
        }
        return $res;
    }

    function readResponse() {
        $res = $this->readMessage();
        if (empty($res['result']) && empty($res['error'])) {
            //throw new RuntimeException('Bad editor response');
        }
        return $res;
    }

    function readRequest() {
        $res = $this->readMessage();
        if (empty($res['method'])) {
            throw new RuntimeException('Bad editor request');
        }
        if (empty($res['params'])) {
            $res['params'] = [];
        }
        return $res;
    }

    function send($arr) {
        if (empty($arr['id'])) {
            $arr['id'] = uniqid();
        }
        $qstr = http_build_query($arr);
        $qstr = preg_replace('@(?<=params%5B)\d+(?=%5D)@', '', $qstr);
        echo $qstr . "\n";
        ob_flush();
        flush();
        return $arr['id'];
    }
}

class EOFException extends RuntimeException {
}

class Test extends MleUscript {
    function getCommands() {
        return [
            'test',
        ];
    }

    function handleRequest($req) {
        $this->mark_insert_before($req['params']['mark'], "hello", strlen("hello"));
        return MLE_OK;
    }
}

(new Test())->run();
