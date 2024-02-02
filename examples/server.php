<?php

//Dialplan should contain
//<action application="socket" data="192.168.0.22:8084 async full"/>

$server = null;

try {
    $serv = new ESLserver("192.168.0.22", 8084);
} catch (Exception $e) {
    print $e->getMessage() . PHP_EOL;
}

if(!$server) die("cannot connect");

while(true) {
    $new_sock = $serv->accept();

    $pid = pcntl_fork();

    if ($pid == -1) {
         die('could not fork');
    } else if ($pid) {
         // we are the parent
         pcntl_wait($status); //Protect against Zombie children
    } else {
         // we are the child
         if($new_sock) {
                 $esl = new ESLconnection($new_sock);   //HERE WE GO!

                 $ev = $esl->getInfo();         //This is the first event received in outbound mode
                 print_r($ev->serialize());     //containing CHANNEL_DATA

                 $uid = $ev->getHeader("Unique-ID");
                 $esl->execute("answer", "", $uid);
                 //$esl->execute("playback", "/path/to/we_are_trying_to_reach.wav", $uid);
                 sleep(5);
                 $esl->execute("hangup", "", "");
                 $serv->close($new_sock);       //User is responsible for closing the new child socket
                 exit;                          //Make sure to exit the fork
             }
    }
}