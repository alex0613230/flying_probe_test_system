<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>系統管理功能</title>
<style>
	.chi {font-family:"標楷體";font-size:20px;}
	th, button, input {font-size:24px;}
</style>
<link rel="shortcut icon" href="favicon.ico" type="image/x-icon" />
</head>
<?	
	if (isset($_POST["reset"]))
	{
		$message = "0110001F0001020001";//重置

		// 送出訊息
		$str = "./uart " . $message;
		$response = exec($str, $output, $ret);
		
		$error = 1;
		if (($response[2] == '1') && ($response[3] == '0'))
			$error = 0;
	}
?>

<body bgcolor="#D4FFFF">
<center>
<table border=5 cellspacing=0 bordercolor="#6363FF" cellpadding=10 width=50%>
<tr>
	<th height=20 colspan=3><font size=7>開　發　選　項</font></th>
</tr>

<tr>
	<th>
		<a href="./Command.php"><img src="./pic/Wi.png" style="width:160px;height:150px;"></a><br>Wi_SUN 命令
	</th>
	<th>
		<a href="./Crane.php"><img src="./pic/truck.png" style="width:160px;height:150px;"></a><br>天車操作

	</th>
</tr>
<tr>
	<th>
		<a href="./Warehouse_address.php"><img src="./pic/warehouse.png" style="width:160px;height:150px;"></a><br>新增倉儲地址
	</th>
	<th>
		<a href="./work.php"><img src="./pic/work.png" style="width:160px;height:150px;"></a><br>使用者管理
	</th>
</tr>
</table><p>

<table border=5 cellspacing=0 bordercolor="#6363FF" cellpadding=10 width=50%>
<tr>
	
	<th bgcolor=red>
		<button onclick="location.href='./index.php'" style="width:180px;height:40px;">一般介面</button>
	</th>

</tr>

<?
	if (isset($_POST["reset"]))
	{
		if ($error == 0)
			echo "<tr><th colspan=2><font color=blue>ESP32 重置成功</font></th></tr>";
		else	echo "<tr><th colspan=2><font color=red>ESP32 重置失敗</font></th></tr>";
	}
?>

</table>
</center>
</body>
</html>
