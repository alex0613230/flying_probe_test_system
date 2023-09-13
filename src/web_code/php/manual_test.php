<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>手動測試功能</title>
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
	
	$fpp = 0;

	if (isset($_POST["action"]))
	{
		if (isset($_POST["set_cur_point"]))	// 設定測試點位編號
		{
			$message = "0110001F000204";
			$op_scp = $_POST["set_cur_point"];
			$op_scp_data = $_POST["set_No"];
			
			$message .= sprintf("%04X", $op_scp);
			$message .= sprintf("%04X", $op_scp_data);
			
			$fpp = 1;
		}
		
		if (isset($_POST["manual_esp_op"]))	// 執行目前點位測試
		{
			$message = "0110001F000204000B0000";
		}
		
		if (isset($_POST["manual_op_one"]))	// 執行上/下一點位測試
		{
			$message = "0110001F000204";
			$addr = $_POST["manual_op_one"];
			
			$message .= sprintf("%04X", $addr);
			$message .= sprintf("%04X", 1);
		}
		
		if (isset($_POST["manual_op"]))		// 執行相對點位測試
		{
			$message = "0110001F000204";
			$addr = $_POST["manual_op"];
			
			$message .= sprintf("%04X", $addr);
			
			if (isset($_POST["No_Point"]))
				$pp = $_POST["No_Point"] + 0;
			else	$pp = 1;
			
			$message .= sprintf("%04X", $pp);
		}
		
		$str = "./uart " . $message;
		$response = exec($str, $output, $ret);
			
		$error = 1;
		if (($response[2] == '1') && ($response[3] == '0'))
			$error = 0;
	}
?>

&nbsp;<br>
<center>
<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=blue>
<tr>
	<th height=30 colspan=3 bgcolor="#66d9ff">
		<font size=6>手　動　測　試　功　能</font>
	</th>
</tr>
</table><p>

<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=blue>
<form action="./manual_test.php" method="POST">
<input type=hidden name=action value=1>
<tr>
	<th>
		設定測試點位編號：<input type=text name=set_No size=5>&emsp;
		<button type="submit" name=set_cur_point value=14 style="width:210px;height:40px;">設定</button>
	</th>
</tr>

<tr>
	<th>
		<button type="submit" name=manual_esp_op value=11 style="width:220px;height:40px;">目前點位測試</button>&emsp;
		<button type="submit" name=manual_op_one value=12 style="width:220px;height:40px;">下一點位測試</button>&emsp;
		<button type="submit" name=manual_op_one value=13 style="width:220px;height:40px;">上一點位測試</button>&emsp;&emsp;
		相對點位測試：<input type=text name=No_Point value=1 size=4>&emsp;
		<button type="submit" name=manual_op value=12 style="width:110px;height:40px;">向下</button>&emsp;
		<button type="submit" name=manual_op value=13 style="width:110px;height:40px;">向上</button>
	</th>	
</tr>	
	
</table><p>

<table border=3 width=80%  cellspacing=2 cellpadding=10 bordercolor=blue>
<tr>
	<th height=30 colspan=4 bgcolor="#66d9ff">
		<font size=6>測　試　狀　況</font>
	</th>
</tr>

<?
	if (isset($_POST["action"]))
	{
		if ($error == 1)
		{
			echo "<tr><th colspan=4><font color=red>命令寫出錯誤，ESP32 回應：" . $response . "</font></th></tr>";
		}
		else
		{
			$tries = 0;
			while ($tries < 15)
			{
				$message = "010300150005";
				$str = "./uart " . $message;
				$response = exec($str, $output, $ret);
				
				$error = 1;
				if (($response[2] == '0') && ($response[3] == '3'))
					$error = 0;
				
				if ($error == 1)
				{
					echo "<tr><th colspan=4><font color=red>資料讀取錯誤，ESP32 回應：" . $response . "</font></th></tr>";
					break;
				}
				else
				{
					$Total_Point_2 = to_binary($response, 6, 9);
					$Cur_Point_2 = to_binary($response, 10, 13);
					$Single_Result = to_binary($response, 14, 17);
					$Single_Function = to_binary($response, 18, 21);
					$Single_Status = to_binary($response, 22, 25);
					
					if ($fpp == 0)
					{
						if ($Single_Status == 1)
						{
							$tries ++;
							sleep(1);
							continue;
						}

						echo "<tr><th width=25%>測試總點數</th><th width=25%>正被測試的點位編號</th><th width=25%>測試功能</th><th width=25%>測試結果</th></tr>";
						echo "<tr><th>$Total_Point_2</th><th>$Cur_Point_2</th>";
						
						switch ($Single_Function)
						{
							case 0:	$pp1 = "GND 短路測試";
								if ($Single_Result == 0)
									$pp2 = "<font color=blue>" . $Single_Result . " 成功</font>";
								else	$pp2 = "<font color=red>" . $Single_Result . " 失敗</font>";
								break;
								
							case 1: $pp1 = "Vss 短路測試";
								if ($Single_Result == 0)
									$pp2 = "<font color=blue>" . $Single_Result . " 成功</font>";
								else	$pp2 = "<font color=red>" . $Single_Result . " 失敗</font>";
								break;
								
							case 2: $pp1 = "類比測試";
								$pp2 = $Single_Result;
								break;
						}
						
						echo "<th> $pp1 </th>";
						echo "<th> $pp2 </th>";
					}
					else
					{
						echo "<tr><th width=25%>測試總點數</th><th width=25%>正被測試的點位編號</th><th width=25%>測試功能</th><th width=25%>測試結果</th></tr>";
						echo "<tr><th>$Total_Point_2</th><th>$Cur_Point_2</th>";
						echo "<th>--</th><th>--</th>";		// 只是設定測試點位，沒有執行測試作業
					}
					
					echo "</th></tr>";
					
					break;
				}
			}
			
			if ($tries == 10)
			{
				echo "<tr><th colspan=4><font color=red>作業時間超過 15 秒鐘，請檢查系統或測試資料！</font></th></tr>";
			}
		}
	}
	else
	{
		$message = "010300150001";
		$str = "./uart " . $message;
		$response = exec($str, $output, $ret);
		
		$error = 1;
		if (($response[2] == '0') && ($response[3] == '3'))
			$error = 0;
		
		if ($error == 1)
		{
			echo "<tr><th colspan=4><font color=red>資料讀取錯誤，ESP32 回應：" . $response . "</font></th></tr>";
		}
		else
		{
			$Total_Point_2 = to_binary($response, 6, 9);

			if ($Total_Point_2 == 0)
				echo "<tr><th colspan=4><font color=red>機台測試資料不存在，無法執行！</font></th></tr>";
			else
			{
				echo "<tr><th width=25%>測試總點數</th><th width=25%>正被測試的點位編號</th><th width=25%>測試功能</th><th width=25%>測試結果</th></tr>";
				echo "<tr><th>$Total_Point_2</th><th>--</th><th>--</th><th>--</th></tr>";
			}
		}

	}
?>
</form>
</table><br>

<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=blue>
<tr>
	<th><a href="./index.php">返回首頁</a></th>
</tr>
</table>

</center>
</body>
</html>