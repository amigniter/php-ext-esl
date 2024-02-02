<?php

try {
    $conn = new ESLconnection("localhost", "8021", "ClueCon");
} catch (Exception $e) {
    print $e->getMessage() . PHP_EOL;
}

$e = $conn->api("status");

if($e) {
    print_r( $e->serialize() );
}