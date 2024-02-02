<?php

$event = new ESLevent("SEND_MESSAGE");
$event->addHeader("User", "1001");
$event->addHeader("Profile", "internal");
$event->addHeader("Host", "localhost");
$event->addBody("this is a body!");

$priority = $event->setPriority("HIGH"); // LOW,HIGH,NORMAL
//var_dump($priority);

$con = new ESLConnection("localhost", "8021", "ClueCon");
$con->sendEvent($event);

