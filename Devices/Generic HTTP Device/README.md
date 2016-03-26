# Generic HTTP Device

This project consists of a Raspberry Pi running Raspbian OS. It runs HTTPD with index.php as the source. The PHP runs the gpio command in order to enable the pins on the Pi. The code sample in the PHP file causes a relay to momentarily turn on then off. I'm using this on a gate so the short/momentary capability was key.

The Groovy file is the Device Handler for SmartThings.
index.php is meant to reside in /var/www/html folder of the Raspbery Pi and runs the external gpio command in Linux.

This is the original video that I used for the Raspberry Pi piece: https://www.youtube.com/watch?v=p2abZ90-eU0

The SmartThings driver was a few hours of self-learning and a bit of frustration --- I really wanted that value tile to show me the last time the script ran WITH SUCCESS.