<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>手動操作功能</title>
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
<body>

&nbsp;<br>
<form action="./manual_op.php" method="POST">
<input type=hidden name=action value=1>
<center>
<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
<tr>
	<th height=30 colspan=2 bgcolor="#66d9ff">
		<font size=6>手　動　操　作　功　能</font>
	</th>
</tr>
</table><p>

<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
<form action="./manual_op.php" method="POST">
<input type=hidden name=action value=1>
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
	
	$error = 0;
	
	if (isset($_POST["action"]))
	{
		if (isset($_POST["gohome"]))
		{
			$message = "0110001F0001020002";	// 步進馬達歸位
		}
		else if (isset($_POST["Motor_move_dis"]))	// 步進馬達移動，固定距離
		{
			$message = "0110001F000204";
			$dis = $_POST["Motor_move_dis"];
			switch ($dis)
			{
				case 0: //100 mm X_forward
					$message .= sprintf("%04X", 21);
					$message .= sprintf("%04X", 10000);
					break;
					
				case 1: //10 mm X_forward
					$message .= sprintf("%04X", 21);
					$message .= sprintf("%04X", 1000);
					break;
					
				case 2: //1 mm X_forward
					$message .= sprintf("%04X", 21);
					$message .= sprintf("%04X", 100);
					break;
					
				case 3: //100 mm X_backward
					$message .= sprintf("%04X", 22);
					$message .= sprintf("%04X", 10000);
					break;
					
				case 4: //10 mm X_backward
					$message .= sprintf("%04X", 22);
					$message .= sprintf("%04X", 1000);
					break;
					
				case 5: //1 mm X_backward
					$message .= sprintf("%04X", 22);
					$message .= sprintf("%04X", 100);
					break;
					
				case 6: //100 mm Y_forward
					$message .= sprintf("%04X", 23);
					$message .= sprintf("%04X", 10000);
					break;
					
				case 7: //10 mm Y_forward
					$message .= sprintf("%04X", 23);
					$message .= sprintf("%04X", 1000);
					break;	
					
				case 8: //1 mm Y_forward
					$message .= sprintf("%04X", 23);
					$message .= sprintf("%04X", 100);
					break;
					
				case 9: //100 mm Y_backward
					$message .= sprintf("%04X", 24);
					$message .= sprintf("%04X", 10000);
					break;
					
				case 10: //10 mm Y_backward
					$message .= sprintf("%04X", 24);
					$message .= sprintf("%04X", 1000);
					break;
					
				case 11: //1 mm Y_backward
					$message .= sprintf("%04X", 24);
					$message .= sprintf("%04X", 100);
					break;	
					
				case 12: //10 mm Z_UP
					$message .= sprintf("%04X", 25);
					$message .= sprintf("%04X", 1000);
					break;
					
				case 13: //1 mm Z_UP
					$message .= sprintf("%04X", 25);
					$message .= sprintf("%04X", 100);
					break;	
					
				case 14: //0.5 mm Z_UP
					$message .= sprintf("%04X", 25);
					$message .= sprintf("%04X", 50);
					break;
					
				case 15: //10 mm Z_DOWN
					$message .= sprintf("%04X", 26);
					$message .= sprintf("%04X", 1000);
					break;
					
				case 16: //1 mm Z_DOWN
					$message .= sprintf("%04X", 26);
					$message .= sprintf("%04X", 100);
					break;
					
				case 17: //0.5 mm Z_DOWN
					$message .= sprintf("%04X", 26);
					$message .= sprintf("%04X", 50);
					break;
			}
		}
		else if (isset($_POST["Motor_move_dir"]))	// 步進馬達移動，手動輸入距離
		{
			$message = "0110001F000204";
			$set_motor = $_POST["Motor_move_dir"];
			$dir = $_POST["Motor"] + $set_motor;
			$set_dis = $_POST["Set_Motor_dis"] * 100;
			
			$message .= sprintf("%04X", $dir);
			$message .= sprintf("%04X", $set_dis);
		}

		// 送出訊息
		$str = "./uart " . $message;
		$response = exec($str, $output, $ret);
		
		$error = 1;
		if (($response[2] == '1') && ($response[3] == '0'))
			$error = 0;
	}
?>

<tr>
	<th colspan=2>
		<button type="submit" name="gohome" value=0 style="width:180px;height:40px;">步進馬達歸位</button>&nbsp;&nbsp;&nbsp;&nbsp;
	</th>
</tr>

<tr>
	<th>X 軸馬達</th>
	<th>
		<button type=submit name=Motor_move_dis value=0 style="width:180px;height:40px;">前進 100 mm</button>&nbsp;
		<button type=submit name=Motor_move_dis value=1 style="width:180px;height:40px;">前進 10 mm</button>&nbsp;
		<button type=submit name=Motor_move_dis value=2 style="width:180px;height:40px;">前進 1 mm</button>&emsp;
		
		<button type=submit name=Motor_move_dis value=3 style="width:180px;height:40px;">100 mm 後退</button>&nbsp;
		<button type=submit name=Motor_move_dis value=4 style="width:180px;height:40px;">10 mm 後退</button>&nbsp;
		<button type=submit name=Motor_move_dis value=5 style="width:180px;height:40px;">1 mm 後退</button>
	</th>
</tr>	

<tr>
	<th>Y 軸馬達</th>
	<th>
		<button type=submit name=Motor_move_dis value=6 style="width:180px;height:40px;">前進 100 mm</button>&nbsp;
		<button type=submit name=Motor_move_dis value=7 style="width:180px;height:40px;">前進 10 mm</button>&nbsp;
		<button type=submit name=Motor_move_dis value=8 style="width:180px;height:40px;">前進 1 mm</button>&emsp;
		
		<button type=submit name=Motor_move_dis value=9 style="width:180px;height:40px;">100 mm 後退</button>&nbsp;
		<button type=submit name=Motor_move_dis value=10 style="width:180px;height:40px;">10 mm 後退</button>&nbsp;
		<button type=submit name=Motor_move_dis value=11 style="width:180px;height:40px;">1 mm 後退</button>
	</th>
</tr>

<tr>
	<th>Z 軸馬達</th>
	<th>
		<button type=submit name=Motor_move_dis value=12 style="width:180px;height:40px;">向下 10 mm</button>&nbsp;
		<button type=submit name=Motor_move_dis value=13 style="width:180px;height:40px;">向下 1 mm</button>&nbsp;
		<button type=submit name=Motor_move_dis value=14 style="width:180px;height:40px;">向下 0.5 mm</button>&emsp;
		
		<button type=submit name=Motor_move_dis value=15 style="width:180px;height:40px;">10 mm 向上</button>&nbsp;
		<button type=submit name=Motor_move_dis value=16 style="width:180px;height:40px;">1 mm 向上</button>&nbsp;
		<button type=submit name=Motor_move_dis value=17 style="width:180px;height:40px;">0.5 mm 向上</button>
	</th>
</tr>

<tr>
	<th>步進馬達設定</th>
	<th>
		<input type=radio name=Motor value=21> X 軸
		<input type=radio name=Motor value=23> Y 軸
		<input type=radio name=Motor value=25> Z 軸
		&nbsp;&nbsp;
		<input type=text name=Set_Motor_dis size=8>&nbsp;mm
		&nbsp;&emsp;
		<button type=submit name=Motor_move_dir value=0 style="width:140px;height:40px;">前進/向下</button>&nbsp;&emsp;
		<button type=submit name=Motor_move_dir value=1 style="width:140px;height:40px;">後退/向上</button>
	</th>

</tr>
</form>
</table><br>

<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
<tr>
	<th height=30 colspan=3 bgcolor="#66d9ff">
		<font size=6>步　進　馬　達　目　前　絕　對　位　置</font>
	</th>
</tr>
<tr>
        <th>X 軸</th>
        <th>Y 軸</th>
        <th>Z 軸</th>
</tr>

<?
	if ($error == 1)
	{
		echo "<tr><th colspan=3><font color=red>命令寫出錯誤，ESP32 回應：" . $response . "</font></th></tr>";
	}
	else
	{
		// 讀取目前位置
		$message = "0103000F0003";
		$str = "./uart " . $message;
		$response = exec($str, $output, $ret);
		
		$error = 1;
		if (($response[2] == '0') && ($response[3] == '3'))
			$error = 0;
		
		if ($error == 1)
		{
			echo "<tr><th colspan=3><font color=red>資料讀取錯誤，ESP32 回應：" . $response . "</font></th></tr>";
		}
		else
		{
			$x = to_binary($response, 6, 9) / 100.0;
			$y = to_binary($response, 10, 13) / 100.0;
			$z = to_binary($response, 14, 17) / 100.0;
			
			echo "<tr><th>$x</th><th>$y</th><th>$z</th></tr>";
		}
	}
?>

</table><br>

<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
<tr>
	<th><a href="./index.php">返回首頁</a></th>
</tr>
</table>
</center>

</body>
</html>