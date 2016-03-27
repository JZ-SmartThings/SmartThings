<?php
$perform_authentication=false;

if ($perform_authentication) {
	$valid_passwords = array ("gate" => "gate1");
	$valid_users = array_keys($valid_passwords);
	$user = $_SERVER['PHP_AUTH_USER'];
	$pass = $_SERVER['PHP_AUTH_PW'];
	$validated = (in_array($user, $valid_users)) && ($pass == $valid_passwords[$user]);
	if (!$validated) {
	  header('WWW-Authenticate: Basic realm="Generic HTTP Device"');
	  header('HTTP/1.0 401 Unauthorized');
	  die ("Authentication Required!");
	}
}
// If code arrives here, this would be a valid user.
?>

<html>
<head>
<meta charset="UTF-8" />
<!--  <link rel="stylesheet" type="text/css" href="css/style.css"> -->
<meta name=viewport content='width=700'>

<style type='text/css'>
body, pre	 {
	max-width: 640px;
	margin: 0 auto;
	font-family: Calibri,Arial,Helvetica,sans-serif;
	background-color: #E3E3E3;
	font-size: 1.2em;
	line-height: 1.5em;
}
.btn {
	font-family: 'Open Sans';
	font-weight: bold;
	font-size: 1.2em;
	foreground-color: white;
	line-height: 4em;
	margin: 10px 0px;
	width: 240px;
	border-top: 1px solid #969696;
	background: #000000;
	background: -webkit-gradient(linear, left top, left bottom, from(#545454), to(#000000));
	background: -webkit-linear-gradient(top, #545454, #000000);
	background: -moz-linear-gradient(top, #545454, #000000);
	background: -ms-linear-gradient(top, #545454, #000000);
	background: -o-linear-gradient(top, #545454, #000000);
	padding: 5px 10px;
	-webkit-border-radius: 8px;
	-moz-border-radius: 8px;
	border-radius: 8px;
	-webkit-box-shadow: rgba(0,0,0,1) 0 1px 0;
	-moz-box-shadow: rgba(0,0,0,1) 0 1px 0;
	box-shadow: rgba(0,0,0,1) 0 1px 0;
	text-shadow: rgba(0,0,0,.4) 0 1px 0;
	color: #e3e3e3 !important;
	text-decoration: none;
	vertical-align: middle;
}
.btn:hover {
   border-top-color: #4f4f4f;
   background: #4f4f4f;
   color: #ccc;
   text-decoration:none;
}
.center {
	margin: auto;
	width: 60%;
	border: 3px solid #000000;
	padding: 10px;
	text-align: center;
}
</style>

</head>

<body>

<div class="center">
<pre>
<?php
//DATE
date_default_timezone_set('America/Los_Angeles');
echo "Date=".date("M j<\s\u\p>S</\s\u\p\>, Y h:i:s A"."\n"); 

//SPACE USED
$output = shell_exec('df -h|grep /dev/root | cut -d \' \' -f 14');
echo "Space Used=$output";

//CPU
//$output = shell_exec('top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk \'{print 100 - $1"%"}\'');
$output = shell_exec('grep \'cpu \' /proc/stat | awk \'{usage=($2+$4)*100/($2+$4+$5)} END {print usage "%"}\' | sed \'s/\(\.[0-9]\).*$/\1%/g\'');
echo "CPU=$output";

//UPTIME
$output = shell_exec('uptime');
$firstpos = strpos($output, 'up');
$lastpos = strpos($output, ',');
$output = trim(substr($output,$firstpos+2, $lastpos-$firstpos-2));
echo "UpTime=$output\n";

//TEMPERATURE
$tempcelcius = shell_exec('sudo vcgencmd measure_temp | sed "s/temp=//g" | sed "s/\'/°/g"');
$tempfahr = round(trim(substr($tempcelcius,0, strpos($tempcelcius, '°')-6))*9/5+32,1);
$tempfahr = 'CPU Temp=' . $tempfahr . '°F';
echo "CPU Temp=$tempcelcius$tempfahr\n";

if (isset($_POST['GateTrigger']))
{
//exec("sudo python /var/www/momentary.py");
exec("sudo gpio -g mode 4 out ; gpio -g write 4 1 ; sleep 1 ; gpio -g write 4 0");
echo "GateTrigger=Success\n";
}
if (isset($_POST['Test']))
{
echo "Test=Success\n";
}
if (isset($_POST['RebootNow']))
{
exec("sudo shutdown -r now");
}
?>
</pre>


<form method="post">
	<button class="btn" name="GateTrigger">Gate Trigger</button>
	<br/>
	<button class="btn" name="RebootNow" OnClick='return (confirm("Are you sure you want to reboot?"));'>Reboot Now</button>
	<br/>
	<button class="btn" name="Test">Test</button>
</form>
</div>
</body>


</html>