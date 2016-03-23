<html>
<head>
<meta charset="UTF-8" />
  <link rel="stylesheet" type="text/css" href="css/style.css">
</head>


<?php
if (isset($_POST['GateTrigger']))
{
exec("sudo python /var/www/momentary.py");
echo "Success";
}
if (isset($_POST['RebootNow']))
{
exec("sudo shutdown -r now");
}
if (isset($_POST['Test']))
{
echo "Success";
}
?>



<form method="post">
<button class="btn" name="GateTrigger">Gate Trigger</button>&nbsp;
<br/>
<button class="btn" name="RebootNow">Reboot Now</button>&nbsp;
<br/>
<button class="btn" name="Test">Test</button>&nbsp;
</form>


</html>