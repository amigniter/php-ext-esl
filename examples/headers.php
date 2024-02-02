<?php

$event = new ESLevent("SEND_MSG");
$event->addHeader("User", "1001");
$event->addHeader("Profile", "internal");
$event->addHeader("Host", "localhost");
$event->addBody("this is a body!");

/*
//here we connect and fire event to FreeSWITCH event system
$con = new ESLConnection("localhost","8021","ClueCon");
$con->sendEvent($event);
*/

$header = $event->firstHeader(); //first set the pointer to first header
echo "firstHeader: $header\n";

//loop through all the headers
while ($header !== null) {
    echo "Header: $header\n";
    $header = $event->nextHeader();
}

//delete a header
$event->delHeader("Host");