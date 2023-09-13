<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title>詳細測試結果資料</title>
<style>
	a:hover {background-color:blue;color:white;}
	a {text-decoration:none;}
	.chi {font-family:"標楷體";font-size:24px;}
	th, button, input {font-size:24px;}
</style>
</head>
<body>

&nbsp;<br>
<center>
<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
<tr>
	<th height=30 colspan=2 bgcolor=yellow>
		<font size=6>詳　細　測　試　結　果</font>
	</th>
</tr>
</table><p>

<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
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
	
	$message = "0103000B0004";
	$str = "./uart " . $message;
	$response = exec($str, $output, $ret);
	
	$error = 1;
	if (($response[2] == '0') && ($response[3] == '3'))
		$error = 0;
	
	$Total_Point = 0;
	if ($error == 1)
	{
		echo "<tr><th><font color=red>資料讀取錯誤，ESP32 回應：" . $response . "</font></th></tr>";
	}
	else
	{
		$Total_Point = to_binary($response, 6, 9) ;
		
		echo "<tr><th>測試總點數：" . $Total_Point . "</th></tr>";
	}
	
	if ($Total_Point > 0)
	{
		//讀取自動測試資料
		$START_ADDR = 7001; //存放測試結果起始位置
		$count_Q = floor($Total_Point / 10);
		$count_R = $Total_Point % 10;
		
		$error = 0;
		$index = 0;
		
		$result = array();
		
		for ($i = 0; $i < $count_Q; $i ++)
		{
			$message = "0103";
			$message .= sprintf("%04X", $START_ADDR);
			$message .= sprintf("%04X", 10);
			
			$str = "./uart " . $message;
			$response = exec($str, $output, $ret);
			
			$START_ADDR += 10;
			
			if (($response[2] != '0') || ($response[3] != '3'))
			{
				$error = 1;
				break;
			}
			
			for ($j = 0; $j < 10; $j ++)
			{
				$result[$index] = to_binary($response, 6 + (4 * $j), 9 + (4 * $j));
				$index ++;
			}
		}
		
		if ($error == 0)
		{
			if ($count_R > 0)
			{
				$message = "0103";
				$message .= sprintf("%04X", $START_ADDR);		
				$message .= sprintf("%04X", $count_R);
			
				$str = "./uart " . $message;
				$response = exec($str, $output, $ret);
								
				if (($response[2] != '0') || ($response[3] != '3'))
				{
					$error = 1;
				}
				else
				{
					for ($j = 0; $j < $count_R; $j ++)
					{
						$result[$index] = to_binary($response, 6 + (4 * $j), 9 + (4 * $j));
						$index ++;
					}
				}
			}
		}

		if ($error == 1)
		{
			echo "<tr><th><font color=red>資料讀取錯誤，ESP32 回應：" . $response . "</font></th></tr>";
		}	
		else
		{
			$line = 20;
			echo "<tr><th align=center>";
			echo "<table border=2 width=800>";
			echo "<tr><th width=20%></th>";
			for ($i = 1; $i <= $line; $i ++) //標示 1 - 20
			{	
				echo "<th width=4%>$i</th>";
			}
			echo "</tr>";
			
			$i = 0;
			while ($i < $index)
			{
				if ($i % $line == 0)
				{
					echo "<tr>";
					$start = floor($i / 10) * 10 + 1;
					$end = floor($i / 10) * 10 + $line;
					echo "<th>" . $start . " - " . $end . "</th>";
				}
				
				if ($result[$i] == 0)
					echo "<th bgcolor=blue>&nbsp;</th>";	//成功顯示藍色
				else	echo "<th bgcolor=red>&nbsp;</th>";	//失敗顯示紅色
				
				$i ++;
				if ($i % $line == 0)
					echo "</tr>";
			}
			
			if ($i % $line != 0)
			{
				while ($i % $line != 0)
				{
					echo "<th>&nbsp;</th>";
					$i ++;
				}
				echo "</tr>";
			}
				
			echo "</table>";
			echo "</th></tr>";
		}
	}
?>

</table><br>

<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=blue>
<tr>
	<th><a href="./auto_test.php">返回上一頁</a></th>
</tr>
</table>
</center>

</body>
</html>