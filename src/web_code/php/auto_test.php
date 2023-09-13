<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>自動測試功能</title>
<style>
	a:hover {background-color:blue;color:white;}
	a {text-decoration:none;}
	.chi {font-family:"標楷體";font-size:24px;}
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
</head>

<?
	function to_binary($buf, $start, $end)
	{
		$data = 0;
		for ($i = $start; $i <= $end; $i ++)
		{
			if (($buf[$i] >= '0') && ($buf[$i] <= '9'))
				$data = $data * 16 + (ord($buf[$i]) - ord('0'));
			else if (($buf[$i] >= 'A') && ($buf[$i] <= 'F'))
				$data = $data * 16 + (ord($buf[$i]) - ord('A') + 10);
		}
		
		return $data;
	}
	
	header("Refresh:5;");
?>

<body>

&nbsp;<br>
<center>
<table bgcolor=white width=80% style="border-radius:20 px" >
<tr>
	<th bgcolor="#66d9ff">
		<font size=6>自　動　測　試　功　能</font>
	</th>
</tr>
</table><p>

<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
<form action="./auto_test.php" method="POST">
<input type=hidden name=action value=1>

<tr>
	<th>
		<button type="submit" name="auto_esp_op" value=3 style="width:180px;height:40px;">START</button>&emsp;
		<button type="submit" name="auto_esp_op" value=4 style="width:180px;height:40px;">PAUSE</button>&emsp;
		<button type="submit" name="auto_esp_op" value=5 style="width:180px;height:40px;">CONTINUE</button>&emsp;
		<button type="submit" name="auto_esp_op" value=6 style="width:180px;height:40px;background-color:#FF0000;">STOP</button>
	</th>
</tr>
</form>

<?
	if (isset($_POST["auto_esp_op"]))
	{
		$message = "0110001F000102";
		$auto_opcode = $_POST["auto_esp_op"];
		$message .= sprintf("%04X", $auto_opcode);
		
		$str = "./uart " . $message;

		$response = exec($str, $output, $ret);
		
		$error = 1;
		if (($response[2] == '1') && ($response[3] == '0'))
			$error = 0;		

		if ($error == 1)
			echo "<tr><th><font color=red>命令寫出錯誤，ESP32 回應：" . $response . "</font></th></tr>";
	}
?>

</table><p>

<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
<tr>
	<th bgcolor="#66d9ff">
		<font size=6>ESP32　狀　態</font>
	</th>
</tr>

<?
	$Total_Point = 0;
	$Cur_Point = 0;
	$Pass_Point = 0;
	$Complete_Point = 0;
	$Error_Point = 0;
		
	//Define 狀態資料
	$esp_status = array("閒置", "手動測試", "自動測試", "暫停作業", "馬達警報");
	
	$message = "0103000A0005";
	$str = "./uart " . $message;
	$response = exec($str, $output, $ret);
		
	$error = 1;
	if (($response[2] == '0') && ($response[3] == '3'))
		$error = 0;
		
	if ($error == 1)
	{
		echo "<tr><th><font color=red>資料讀取錯誤，ESP32 回應：" . $response . "</font></th></tr>";
	}
	else
	{
		$status = to_binary($response, 6, 9);
		echo "<tr><th>" . $esp_status[$status] . "</th></tr>"; 
	}
?>
</table><p>

<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
<tr>
	<th height=30 colspan=8 bgcolor="#66d9ff">
		<font size=6>測　試　狀　況</font>
	</th>
</tr>
<tr>
        <th>測試總點數</th>
        <th>正被測試的點編號</th>
	<th>成功點數量</th>
        <th>錯誤點數量</th>
	<th>完成點數量</th>
	<th>完成率</th>
	<th>成功率</th>
	<th>失敗率</th>
</tr>

<?
	if ($error == 0)
	{
		$Total_Point = to_binary($response, 10, 13) ;
		$Cur_Point = to_binary($response, 14, 17);
		$Pass_Point = to_binary($response, 18, 21);
		$Complete_Point = to_binary($response, 22, 25);
		$Error_Point = $Complete_Point - $Pass_Point;
		
		if ($Complete_Point == 0)
		{
			$Comp_rate = "--";
			$Pass_rate = "--";
			$Fail_rate = "--";
		}
		else
		{
			$Comp_rate = number_format(($Complete_Point / $Total_Point)* 100, 2);
			$Pass_rate = number_format(($Pass_Point / $Complete_Point)* 100, 2);
			$Fail_rate = number_format((100 - $Pass_rate), 2);
			$Comp_rate .= "%";
			$Pass_rate .= "%";
			$Fail_rate .= "%";
		}
		
		echo "<tr><th>$Total_Point</th><th>$Cur_Point</th><th>$Pass_Point</th><th>$Error_Point</th><th>$Complete_Point</th><th>$Comp_rate</th><th>$Pass_rate</th><th>$Fail_rate</th></tr>";
	}
	else	echo "<tr><th>&nbsp;</th><th>&nbsp;</th><th>&nbsp;</th><th>&nbsp;</th><th>&nbsp;</th><th>&nbsp;</th><th>&nbsp;</th><th>&nbsp;</th></tr>";

?>

</table><br>

<?
	if ($Total_Point > 0 && $Complete_Point > 0 && $Total_Point == $Complete_Point)
	{
		echo "<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=blue >";
		echo "<tr>";
		echo "<th width=20% bgcolor=yellow>短路測試結果</th>";
		echo "<th><a href='./point_data1.php'>測試結果資料 (圖型)</a></th>";
		echo "<th><a href='./point_data2.php'>測試結果資料 (表格)</a></th>";
		echo "<th><a href='./point_data3.php'>測試結果資料 (CSV)</a></th>";
		echo "</tr>";
		echo "</table><br>";
		
		echo "<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=blue >";
		echo "<tr>";
		echo "<th width=20% bgcolor=yellow>類比測試結果</th>";
		echo "<th><a href='./point_data4.php'>測試結果資料 (表格)</a></th>";
		echo "<th><a href='./point_data5.php'>測試結果資料 (CSV)</a></th>";
		echo "</tr>";
		echo "</table><br>";
	}
?>

<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
<tr>
	<th><a href="./index.php">返回首頁</a></th>
</tr>
</table>
</center>
</body>
</html>