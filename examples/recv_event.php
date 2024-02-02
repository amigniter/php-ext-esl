<?php

try {
    $conn = new ESLconnection("localhost", "8021", "ClueCon");
} catch (Exception $e) {
    print $e->getMessage() . PHP_EOL;
    die();
}

$ev = $conn->sendRecv("event plain all"); //the same as using events() method
//$ev = $conn->events("plain", "all");

print_r($ev->getHeader("Reply-Text"));

$start_seq = 0;

while(true) {
    $e = $conn->recvEvent();
    if($e) {

        print_r($e->serialize("plain")); //plain|json|xml

        print_r($e->getBody()); //get the body

        $seq= (int)$e->getHeader("Event-Sequence");
        if($start_seq == 0) {
            $start_seq= $seq;
        }

        if(($start_seq + 5) == $seq) {
            die("end of 5 sequences");
        }
    }
}
