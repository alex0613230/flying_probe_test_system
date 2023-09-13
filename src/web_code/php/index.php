<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>系統管理功能</title>
<style>
	.chi {font-family:"標楷體";font-size:20px;}
	th, button, input {font-size:24px;}
	table{
		border-collapse: separate;
		border-spacing: 0;
		border-color:white;
	}
	
	th, tr {
		border: 3px solid ;
		border-color: #b3ecff;
		border-collapse: separate;
		border-spacing: 0;	
	}
	/*第一欄最後列：左上*/
	tr:first-child th:first-child
	{
		border-top-left-radius: 20px;
	}
	/*第一欄最後列：左下*/
	tr:last-child th:first-child
	{
		border-bottom-left-radius: 20px;
	}
	/*最後欄第一列：右上*/
	tr:first-child th:last-child
	{
		border-top-right-radius: 20px;
	}
	/*最後欄第一列：右下*/
	tr:last-child th:last-child
	{
		border-bottom-right-radius: 20px;
	}
	
	
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

<body bgcolor="#FFFFFF">
<center>

<table   style="width:50%">
<tr>
	<th bgcolor="#66d9ff" height=20 colspan=2><font size=7>系　統　管　理　功　能</font></th>
</tr>

<tr>
	
	<th>
		<a href="./auto_test.php"><img src="pixabay_auto.png" style="width:160px;height:150px;"></a><br>自動測試功能
		
	</th>
	
	<th>
		<a href="./manual_test.php"><img src="pixabay_manual.png" style="width:160px;height:150px;"></a><br>手動測試功能
	</th>
	
</tr>
<tr>
	<th>
		<a href="./upload.php"><img src="pixabay_load.png" style="width:160px;height:150px;"></a><br>測試資料載入
	</th>
	<th>
		
		<a href="./manual_op.php"><img src="pixabay_op.png" style="width:160px;height:150px;"></a><br>手動操作功能
	</th>
</tr>
</table>

<table style="width:50%">
<tr>
	<form action="./index.php" method="POST">
	<th bgcolor="#4da6ff" style="border-top-left-radius: 20px;border-bottom-left-radius: 20px;">
		<button type="submit" name="reset" style="width:180px;height:40px;">重置 ESP32</button>
	</th>
	</form>
	
	<form action="./cgi/shutdown.cgi" method="POST">
	<th bgcolor="#4da6ff">
		<button type="submit" name="shutdown" style="width:180px;height:40px;">系統關機</button>
	</th>
	</form>
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
