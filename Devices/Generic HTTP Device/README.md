# Generic HTTP Device

This project consists of a Raspberry Pi running Raspbian OS. It runs HTTPD with index.php as the source. The PHP file can run a Python script in order to trigger the GPIO pins on the Pi. This causes a relay to momentarily turn on then off. I'm using this on a gate so the short/momentary capability was key.

The Groovy file is the Device Handler for SmartThings.
Index.php is meant to reside in /var/www/html folder of the Raspbery Pi
Momentary.py can reside anywhere just make sure the PHP file above is correctly referencing the full path. Inside of this file, there are TWO examples. The first is procedural code and the second loops through the GPIO pin array.

This is the original video that I used for the Raspberry Pi piece: https://www.youtube.com/watch?v=p2abZ90-eU0&feature=youtu.be

The SmartThings driver was a few hours of self-learning and a bit of frustration --- I really wanted that value tile to show me the last time the script ran WITH SUCCESS.