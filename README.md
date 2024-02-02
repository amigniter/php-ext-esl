## PHP ESL

Native PHP 8+ extension to connect to FreeSWITCH using inbound or outbound mode providing all the methods as in FS php esl mod.
Using Event Socket Library is extremely easy especially in outbound mode.

#### About

- Extension developed in cpp using native PHP zend engine.
- Stubs provided for IDE integration
- Linux only

### Installation

- Build Essentials (Debian|Ubuntu) `build-essential` `autoconf` `automake` or equivalent
-  `php-dev` (Debian|Ubuntu) package is required, or equivalent depending on your linux distribution

```bash
phpize
./configure
make
sudo make install
```

### Usage

- For all the methods implemented please visit official FreeSWITCH [Event Socket Library](https://developer.signalwire.com/freeswitch/FreeSWITCH-Explained/Client-and-Developer-Interfaces/Event-Socket-Library/).
- `esl.stub.php` contains the method signatures and can be used as a reference. 

#### Inbound Socket Connection

ESLconnection constructor accepts strings for an inbound connection or integer (socket file descriptor) for an outbound connection.  

A quick example to establish an inbound socket connection
```php
try {
    $con = new ESLconnection("localhost", "8021", "ClueCon");
} catch (Exception $e) {
    print $e->getMessage() . PHP_EOL;
}

$event = $con->api("status");
if($event) {
    print_r($event->serialize());
}
```

Be creative! `examples` folder contains a few more examples to help you get started.

#### Outbound Socket Connection

PHP language in outbound mode has not been particularly easy to use. Native FreeSWITCH PHP ESL mod (which is awesome by the way) supports creating __ESLconnection__ object and using all of its methods. 
Unfortunately PHP itself does not expose raw socket descriptor to php user space which needs to be passed to ESLconnection constructor.
That is not the case any longer with this extension with new `ESLserver` method

An example on how to use ESLserver and all the ESLconnection methods in outbound mode with forking

```php
<?php

$server = null;
try {
    $serv = new ESLserver("localhost", 8084); //8084
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
                 //use wav file to playback on a channel
                 //$esl->execute("playback", "/path/to/we_are_trying_to_reach.wav", $uid);
                 sleep(5);
                 $esl->execute("hangup", "", "");
                 $serv->close($new_sock);       //User is responsible for closing new child socket
                 exit;                          //Make sure to exit the fork
             }
    }
}
```
> **Note**: When in outbound mode the connection is bound to that particular channel thus there is no need to pass uuid as in $esl->execute("answer", "", $uid); Used here for demo purposes (doesn't hurt) to show you how to get it using getHeader("Unique-ID"). It is not passed later in hangup method.



