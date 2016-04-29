# Generic HTTP Device

This project consists of a Raspberry Pi running Raspbian OS. It runs HTTPD with index.php as the source. The PHP runs the gpio command in order to enable the pins on the Pi. The code sample in the PHP file causes a relay to momentarily turn on then off. I'm using this on a gate so the short/momentary capability was key.

Link to the project: https://community.smartthings.com/t/raspberry-pi-to-php-to-gpio-to-relay-to-gate-garage-trigger/43335

The Groovy file is the Device Handler for SmartThings.
index.php is meant to reside in /var/www/html folder of the Raspbery Pi and runs the external gpio command in Linux.
At the top of index.php, change the first variable to "true" instead of "false" and this will make the PHP page protected with basic authentication. After making that change, make sure to change the SmartThings preferences for the device.

This project was tested successfully via an external IP, Pollster and with an Amazon Echo/Alexa. Echo can run TWO functions in my app. The ON command triggers the main function and OFF triggers the custom function.

Here's a useful command for checking the Raspberry Pi pin status & value. It's pretty close to what WebIOPi offers but in text mode and refreshes every half second: watch -n0.5 gpio readall

This is the original video that I used for the Raspberry Pi piece: https://www.youtube.com/watch?v=p2abZ90-eU0

The SmartThings driver was a few hours of self-learning and a bit of frustration --- I really wanted that value tile to show me the last time the script ran WITH SUCCESS.

</br>v1.0.20160428 - Now able to control in preferences whether CustomTrigger is momentary or has states like on/off. This is the CUSTOM tile, which can easily be made as the MAIN/primary tile, search top of ST code for MAIN designation & change to CustomTrigger.
</br>v1.0.20160423 - Small changes: using second line of df -h for space used. Scaling GPIO image better for RPi 3 with more lines on gpio readall command.
</br>v1.0.20160410 - Changing device ID to random number at end of execution so multiple devices can point to one IP. Amazon Echo can run both ON/main and OFF/custom functions. Added ability to output GPIO status in PHP. Releasing another ST device that will pull the image and control the installation of required packages hence the reset of device ID.
</br>v1.0.20160407 - Added Poll & Refresh to execute the TEST function. Now able to add the switch to Pollster and should see the time update in Test Triggered. Validated external IP with custom port and Amazon Echo/Alexa.
</br>v1.0.20160406 - Added the CustomTrigger button. Made buttons smaller. CPU Temp now accurately converts C to F. Added color to some tiles. Defaulting in port & body if left empty in prefs.
</br>v1.0.20160405 - Modified Space Used to use awk instead of cut and delimiters. GitHub reported bug.
</br>v1.0.20160402 - Added JSON support. Modified Reboot tile with states. Added Clear Tiles button.
</br>v1.0.20160328 - Modified tile background while in "running" states and added free memory tile.
</br>v1.0.20160327 - Toggling tile states in general and with respect to authentication. Fixed GPIO in the PHP script to be correct and toggle on then off. Used to have off then on, which is incorrect.
</br>v1.0.20160326 - Added temperature. Added basic authentication.
</br>v1.0.20160323 - Initial version
</br>
<img src="https://raw.githubusercontent.com/JZ-SmartThings/SmartThings/master/Devices/Generic%20HTTP%20Device/Screenshot_Android_App.png">
<img src="https://raw.githubusercontent.com/JZ-SmartThings/SmartThings/master/Devices/Generic%20HTTP%20Device/Screenshot_PHP_Page.png">