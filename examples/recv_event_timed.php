<?php

try {
    $conn = new ESLconnection("localhost", "8021", "ClueCon");
} catch (Exception $e) {
    print_r($e->getMessage());
    die();
}

$conn->sendRecv("event plain all");

/*
while (true) {
    $e = $conn->recvEvent();
    $conn->filter("Event-Name", "HEARTBEAT");       //interested in HEARTBEAT only
    if($e) {
        print_r($e->serialize("xml"));              //xml serialized event
    }
}
*/

while (true) {
    $e = $conn->recvEventTimed(2000); //2s timeout to receive event
    if($e) {
        print_r($e->serialize());
    } else {
        break;
    }
}