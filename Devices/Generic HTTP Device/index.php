<?php //v1.0.20170328 added contact sensor 2

$perform_authentication=false;
$contact_sensor=false;
$contact_sensor_2=false;
$sensor_pin=24;
$sensor_pin_2=25;

if ($perform_authentication) {
	$valid_passwords = array ("gate" => "gate1");
	$valid_users = array_keys($valid_passwords);
	$user = $_SERVER['PHP_AUTH_USER'];
	$pass = $_SERVER['PHP_AUTH_PW'];
	$validated = (in_array($user, $valid_users)) && ($pass == $valid_passwords[$user]);
	if (!$validated) {
		header('WWW-Authenticate: Basic realm="Generic HTTP Device"');
		header('HTTP/1.0 401 Unauthorized');
		if (isset($_POST['Refresh'])) {
			echo "Refresh=Failed : ";
		}
		if (isset($_POST['MainTrigger']) || isset($_POST['MainTriggerOn']) || isset($_POST['MainTriggerOff'])) {
			echo "MainTrigger=Failed : ";
		}
		if (isset($_POST['CustomTrigger']) || isset($_POST['CustomTriggerOn']) || isset($_POST['CustomTriggerOff'])) {
			echo "CustomTrigger=Failed : ";
		}
		if (isset($_POST['RebootNow'])) {
			echo "RebootNow=Failed : ";
		}
		if (isset($_POST['GPIO'])) {
			echo "GPIO=Failed : ";
		}
		if (isset($_POST['GDInstall'])) {
			echo "GDInstall=Failed : ";
		}
		die ("Authentication Required!");
	}
}
// If code arrives here, this would be a valid user.

//BUILD ARRAY VALUES
date_default_timezone_set('America/Los_Angeles');
//CHECK IF GD IS INSTALLED
$gd_installed=false;
foreach(get_loaded_extensions() as $name){ if ($name=='gd') {$gd_installed=true;}}
$rpi = array(
	"Date" => date("Y-m-d h:i:sA"),
	"Space Used" => shell_exec('df -h | awk \'NR==2\' | awk \'{print $(NF-1)}\' | tr -d \'\n\''),
	"UpTime" => trim(substr(shell_exec('uptime'),strpos(shell_exec('uptime'), 'up')+2, strpos(shell_exec('uptime'), ',')-strpos(shell_exec('uptime'), 'up')-2)),
	"CPU" => shell_exec('grep \'cpu \' /proc/stat | awk \'{usage=($2+$4)*100/($2+$4+$5)} END {print usage "%"}\' | sed \'s/\(\.[0-9]\).*$/\1%/g\' | tr -d \'\n\''),
	"CPU Temp" => CPUTemp(),
	"Free Mem" => shell_exec('free -t -h | tr -s " " | grep "Total:" | awk -F " " \'{print $4 " of " $2}\' | tr -d \'\n\''),
	"php5-gd" => $gd_installed
);

function CPUTemp() {
	$celcius = shell_exec('sudo vcgencmd measure_temp | sed "s/temp=//g" | tr -d \'\n\'');
	$fahrenheit = round(substr(str_replace("C","",$celcius), 0, -1) * 1.8 + 32, 0) . "'F";
    return $celcius .' '. $fahrenheit;
}

if ($contact_sensor) {
	shell_exec("sudo gpio -g mode $sensor_pin in");
	$sensor_pin_status = shell_exec("sudo raspi-gpio get $sensor_pin | grep 'func=INPUT' | grep 'level=1'");
	if (strlen($sensor_pin_status) > 5) {
		$rpi = $rpi + array("SensorPinStatus" => 0);
	} else {
		$rpi = $rpi + array("SensorPinStatus" => 1);
	}
} else { // Default to Closed
	$rpi = $rpi + array("SensorPinStatus" => 0);
}
if ($contact_sensor_2) {
	shell_exec("sudo gpio -g mode $sensor_pin_2 in");
	$sensor_pin_status_2 = shell_exec("sudo raspi-gpio get $sensor_pin_2 | grep 'func=INPUT' | grep 'level=1'");
	if (strlen($sensor_pin_status_2) > 5) {
		$rpi = $rpi + array("Sensor2PinStatus" => 0);
	} else {
		$rpi = $rpi + array("Sensor2PinStatus" => 1);
	}
} else { // Default to Closed
	$rpi = $rpi + array("Sensor2PinStatus" => 0);
}

$main_pin=4;
if (isset($_POST['MainPin'])) {
	if (strlen($_POST['MainPin'])>0) { $main_pin=(int)substr(str_replace(";","",$_POST['MainPin']),0,2); }
}
$rpi = $rpi + array("MainPin" => $main_pin);
if (isset($_POST['MainTrigger'])) {
	exec("sudo gpio -g mode $main_pin out ; gpio -g write $main_pin 0 ; sleep 0.1 ; gpio -g write $main_pin 1");
	$rpi = $rpi + array("MainTrigger" => "Success");
}
if (isset($_POST['MainTriggerOn'])) {
	shell_exec("sudo gpio -g mode $main_pin out ; gpio -g write $main_pin 0");
	$rpi = $rpi + array("MainTriggerOn" => "Success");
}
if (isset($_POST['MainTriggerOff'])) {
	shell_exec("sudo gpio -g mode $main_pin out ; gpio -g write $main_pin 1");
	$rpi = $rpi + array("MainTriggerOff" => "Success");
}
$main_pin_status = shell_exec("sudo raspi-gpio get $main_pin | grep 'func=OUTPUT' | grep 'level=1'");
if (strlen($main_pin_status) > 5) {
	$rpi = $rpi + array("MainPinStatus" => 0);
} else {
	$rpi = $rpi + array("MainPinStatus" => 1);
}

$custom_pin=21;
if (isset($_POST['CustomPin'])) {
	if (strlen($_POST['CustomPin'])>0) { $custom_pin=(int)substr(str_replace(";","",$_POST['CustomPin']),0,2); }
}
$rpi = $rpi + array("CustomPin" => $custom_pin);
if (isset($_POST['CustomTrigger'])) {
	shell_exec("sudo gpio -g mode $custom_pin out ; gpio -g write $custom_pin 0 ; sleep 0.1 ; gpio -g write $custom_pin 1");
	$rpi = $rpi + array("CustomTrigger" => "Success");
}
if (isset($_POST['CustomTriggerOn'])) {
	shell_exec("sudo gpio -g mode $custom_pin out ; gpio -g write $custom_pin 0");
	$rpi = $rpi + array("CustomTriggerOn" => "Success");
}
if (isset($_POST['CustomTriggerOff'])) {
	shell_exec("sudo gpio -g mode $custom_pin out ; gpio -g write $custom_pin 1");
	$rpi = $rpi + array("CustomTriggerOff" => "Success");
}
$custom_pin_status = shell_exec("sudo raspi-gpio get $custom_pin | grep 'func=OUTPUT' | grep 'level=1'");
if (strlen($custom_pin_status) > 5) {
	$rpi = $rpi + array("CustomPinStatus" => 0);
} else {
	$rpi = $rpi + array("CustomPinStatus" => 1);
}

if (isset($_POST['Refresh'])) {
	$rpi = $rpi + array("Refresh" => "Success");
}
if (isset($_POST['RebootNow'])) {
	shell_exec("sudo shutdown -r now");
	$rpi = $rpi + array("RebootNow" => "Success");
}
if (isset($_POST['GDInstall']) && $rpi['php5-gd'] != "Installed") {
	$gdinstalling = str_replace("\n","",shell_exec('sudo ps -ef | grep php5-gd | grep -v grep | wc -l'));
	if ($gdinstalling=="0") {
		shell_exec('sudo apt-get update; sudo apt-get -y install php5-gd --fix-missing ; sudo service apache2 restart');
	}
}
if (isset($_POST['GPIO']) && $rpi['php5-gd'] == true) {
	header("Content-type: image/jpeg");
	$gpiolines = shell_exec("gpio readall | wc -l");
	$imheight = 200 + ($gpiolines - 19)*12;
	$im = @imagecreate(400, $imheight) or die("Cannot initialize new GD image stream.");
	$background_color = imagecolorallocate($im, 255, 255, 255);
	$text_color = imagecolorallocate($im, 0, 0, 0);
	$green = imagecolorallocate($im, 51, 255, 153);
	$blue = imagecolorallocate($im, 102, 204, 255);
	$red = imagecolorallocate($im, 255, 51, 51);
	imagefilledrectangle($im, 8, 5, 196, $imheight-5, $green);
	imagefilledrectangle($im, 203, 5, 391, $imheight-5, $blue);
	imagefilledrectangle($im, 172, 0, 227, 11, $red);
	$array = explode("\n", shell_exec("gpio readall"));
	imagestring($im, 1.3, 0, 0, " ", $text_color);
	$counter=0;
	foreach ($array as $v) {
		imagestring($im, 1.3, 0, $counter, $v, $text_color);
		$counter = $counter + 12;
	}
	//imagejpeg($im, NULL, 80-(3*($gpiolines-19)));
	imagejpeg($im, NULL, 90);
	imagedestroy($im);
	exit;
}
if (isset($_POST['UseJSON'])) {
	header('Content-type: application/json');
	echo json_encode($rpi, JSON_PRETTY_PRINT);
	die ();
}
?>

<html>
<head>
<meta charset="UTF-8" />
<meta name=viewport content='width=700'>
<style type='text/css'>
body, pre	 {
	max-width: 640px; margin: 0 auto; font-family: Calibri,Arial,Helvetica,sans-serif;
	background-color: #E3E3E3; font-size: 1.1em; line-height: 1em;
}
.btn {
	font-family: 'Open Sans'; font-size: 1.1em; foreground-color: white;
	line-height: 2.2em; margin: 5px 0px; width: 240px; border-top: 1px solid #969696;
	background: #000000; padding: 3px 5px;
	background: -webkit-gradient(linear, left top, left bottom, from(#545454), to(#000000));
	background: -webkit-linear-gradient(top, #545454, #000000); background: -moz-linear-gradient(top, #545454, #000000);
	background: -ms-linear-gradient(top, #545454, #000000); background: -o-linear-gradient(top, #545454, #000000);
	-webkit-border-radius: 8px; -moz-border-radius: 8px; border-radius: 8px;
	-webkit-box-shadow: rgba(0,0,0,1) 0 1px 0; -moz-box-shadow: rgba(0,0,0,1) 0 1px 0;
	box-shadow: rgba(0,0,0,1) 0 1px 0; text-shadow: rgba(0,0,0,.4) 0 1px 0;
	color: #e3e3e3 !important; text-decoration: none; vertical-align: middle;
}
.btn:hover {
   border-top-color: #4f4f4f; background: #4f4f4f; color: #ccc; text-decoration:none;
}
.center {
	margin: auto; width: 60%; border: 3px solid #000000; padding: 10px; text-align: center;
}
</style>
</head>

<body>
<div class="center">
<pre>
<?php
//DATE
echo "Date=".$rpi['Date']."\n";
//SPACE USED
echo "Space Used=".$rpi['Space Used']."\n";
//CPU
echo "CPU=".$rpi['CPU']."\n";
//UPTIME
echo "UpTime=".$rpi['UpTime']."\n";
//TEMPERATURE
echo "CPU Temp=".str_replace("'","°",$rpi['CPU Temp'])."\n";
//FREE MEMORY
echo "Free Mem=".$rpi['Free Mem']."\n";

echo ($rpi['php5-gd']) ? "php5-gd=Installed\n" : "php5-gd=Not installed\n";

if ($rpi['MainPin']) { echo "MainPin=".$rpi['MainPin']."\n"; }
echo "MainPinStatus=".$rpi['MainPinStatus']."\n";
if ($rpi['CustomPin']) { echo "CustomPin=".$rpi['CustomPin']."\n"; }
echo "CustomPinStatus=".$rpi['CustomPinStatus']."\n";

if ($rpi['MainTrigger']) { echo "MainTrigger=Success\n"; }
if ($rpi['MainTriggerOn']) { echo "MainTriggerOn=Success\n"; }
if ($rpi['MainTriggerOff']) { echo "MainTriggerOff=Success\n"; }
if ($rpi['CustomTrigger']) { echo "CustomTrigger=Success\n"; }
if ($rpi['CustomTriggerOn']) { echo "CustomTriggerOn=Success\n"; }
if ($rpi['CustomTriggerOff']) { echo "CustomTriggerOff=Success\n"; }
echo ($rpi['SensorPinStatus']) ? "SensorPinStatus=Open\n" : "SensorPinStatus=Closed\n";
echo ($rpi['SensorPin2Status']) ? "Sensor2PinStatus=Open\n" : "Sensor2PinStatus=Closed\n";
if ($rpi['Refresh']) { echo "Refresh=Success\n"; }
if ($rpi['RebootNow']) { echo "RebootNow=Success\n"; }
?>
</pre>

<form method="post">
	<button class="btn" name="Refresh">Refresh</button>
	<br/>
	<div style="border: 2px dashed #969696; background-color: #D0D0D0; margin-top: 10px;">
		<button class="btn" style="width: 115px; line-height: 1em;" name="MainTriggerOn">Main Trigger On</button>&nbsp;
		<button class="btn" style="width: 115px; line-height: 1em;" name="MainTriggerOff">Main Trigger Off</button>&nbsp;
		<button class="btn" style="width: 115px; line-height: 1em;" name="MainTrigger">Main Trigger</button>
		<br/>
		<div class="center" style="transform: scale(1.0); -webkit-transform: scale(1.0); margin-top:6px; width:230px;border:1px solid; padding:3px;"><input type="text" name="MainPin" value="4" maxlength="2" size="2">&nbsp;&nbsp;&nbsp;Main Pin # in BCM</div>
		<br/>
	</div>
	<div style="border: 2px dashed #969696; background-color: #D0D0D0; margin-top: 5px;">
		<button class="btn" style="width: 115px; line-height: 1em;" name="CustomTriggerOn">Custom Trigger On</button>&nbsp;
		<button class="btn" style="width: 115px; line-height: 1em;" name="CustomTriggerOff">Custom Trigger Off</button>&nbsp;
		<button class="btn" style="width: 115px; line-height: 1em;" name="CustomTrigger">Custom Trigger</button>
		<br/>
		<div class="center" style="transform: scale(1.0); -webkit-transform: scale(1.0); margin-top:6px; width:230px;border:1px solid; padding:3px;"><input type="text" name="CustomPin" value="21" maxlength="2" size="2">&nbsp;&nbsp;&nbsp;Custom Pin # in BCM</div>
		<br/>
	</div>
	<button class="btn" name="RebootNow" OnClick='return (confirm("Are you sure you want to reboot?"));'>Reboot Now</button>
	<br/>
	<?php if ($rpi['php5-gd'] == "Installed") { ?>
		<button class="btn" name="GPIO" target="_blank">Show GPIO Pins</button>
		<br/>
	<?php } else { ?>
		<button class="btn" name="GDInstall">Install php5-gd</button>
		<br/>
	<?php } ?>
	<div class="center" style="transform: scale(1.3); -webkit-transform: scale(1.3); margin-top:30px; width:150px;border:1px solid;"><input type="checkbox" name="UseJSON" value="">&nbsp;&nbsp;&nbsp;UseJSON</input></div>
	<br/>
</form>
<div>
<a target="_blank" href="https://community.smartthings.com/t/raspberry-pi-to-php-to-gpio-to-relay-to-gate-garage-trigger/43335">Project on SmartThings Community</a></br>
<a target="_blank" href="https://github.com/JZ-SmartThings/SmartThings/tree/master/Devices/Generic%20HTTP%20Device">Project on GitHub</a></br>
</div>

</div>
</body>
</html>
