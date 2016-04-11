# SmartGPIO

This project consists of a Raspberry Pi running Raspbian OS. It runs HTTPD with index.php as the source. The PHP has many functions but this device driver is specifically to see the GPIO status of all pins in a picture format.

Link to the project: <INSERT LINK>

The Groovy file is the Device Handler for SmartThings.
index.php is meant to reside in /var/www/html folder of the Raspbery Pi and runs the a command in Linux and returns a JPEG of that output to SmartThings.
At the top of index.php, change the first variable to "true" instead of "false" and this will make the PHP page protected with basic authentication. After making that change, make sure to change the SmartThings preferences for the device.

Use the TEST button first to make sure that you install a required graphic component for PHP called php5-gd. Once my app senses that this component exists, it will populate the image tile. Otherwise the TAKE button will not work and not return an image as expected.

Here's a useful command for checking the Raspberry Pi pin status & value. It's pretty close to what WebIOPi offers but in text mode and refreshes every half second: watch -n0.5 gpio readall

</br>v1.0.20160410 - Initial version
</br>
<img src="https://github.com/JZ-SmartThings/SmartThings/blob/master/Devices/Generic%20HTTP%20Device/Screenshot_PHP_Page.png">