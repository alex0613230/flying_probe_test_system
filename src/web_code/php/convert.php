<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title>測試結果轉換</title>
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
		<font size=6>測　試　結　果　轉　換</font>
	</th>
</tr>
</table><p>

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
		
		//echo "<tr><th>測試總點數：" . $Total_Point . "</th></tr>";
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
	}
?>


<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
<form action="./convert.php" method="POST">
<input type=hidden name=action value=1>
<tr>
	<th>
		<!-- 輸入電壓值(Vss)&nbsp;<input type=text name=Vss  style=font-size:24px size=4> V&emsp; -->
		參考電阻值(Rf)&nbsp;<input type=text name=Rf  style=font-size:24px size=4>
		<select name=Rf_unit style=font-size:24px>
			<option value=0>mΩ</option>
			<option value=1>Ω</option>
			<option value=2 SELECTED>KΩ</option>
			<option value=3>MΩ</option>
		</select>&emsp;&emsp;
		單位&nbsp;<select name=result_unit style=font-size:24px>
			<option value=0>mΩ</option>
			<option value=1>Ω</option>
			<option value=2 SELECTED>KΩ</option>
			<option value=3>MΩ</option>
		</select>
		&emsp;&emsp;&emsp;&emsp;&emsp;
	</th>	
</tr>
<tr>
	<th>
		<button type=submit name=convert_type_table value=0 style="width:250px;height:40px;">轉換結果資料(表格)</button>&nbsp;
		<button type=submit name=convert_type_csv value=1 style="width:250px;height:40px;">轉換結果資料(CSV)</button>&nbsp;
	</th>
</tr>

<?
	//單位與倍數
	$R_unit = array(0.001, 1, 1000, 1000000);
	$unit_symbol = array('mΩ', 'Ω', 'KΩ', 'MΩ');
	
	//顯示使用者輸入資料
	echo "<tr><th>";
	echo "輸入電壓值(Vss): 3.3";
	echo "　&emsp;參考電阻值(Rf): ". $_POST['Rf'];
	echo $unit_symbol[$_POST['Rf_unit']];
	echo "　&emsp;單位為: ". $unit_symbol[$_POST['result_unit']];
	echo "</th></tr>";		
	
	
	$Vss = 3.3;	//輸入電壓
	$R_ref = $_POST['Rf'] * $R_unit[$_POST['Rf_unit']];	//參考電阻值
	$ADC_offset = 205;	//ADC補償值
		
	if (isset($_POST['action']))
	{	
		if (isset($_POST['convert_type_table']) )	//選擇以表格輸出結果
		{			
			$line = 10;
			echo "<tr><th align=center>";
			echo "<table border=2>";
			echo "<tr>";
					
			for ($i = 1; $i <= $line; $i ++)
			{	
				echo "<th width=5% bgcolor=cyan>點位</th><th width=5%>結果</th>";
			}
			echo "</tr>";
						
			$i = 0;
			while ($i < $index)
			{
				if ($i % $line == 0)
				{
					echo "<tr>";
				}
				
				$j = $i + 1;
				

				echo "<th bgcolor=cyan>$j</th>";
				
				//輸出電壓值換算
				if ($result[$i] + $ADC_offset > 4095)
					$Vout[$i] = 4095 * $Vss / 4096;
				else	$Vout[$i] = ($result[$i] + $ADC_offset) * ($Vss / 4096);
				//電阻值換算				
				$R_load[$i] = ($Vout[$i] / ($Vss - $Vout[$i])) * $R_ref;	
				$R_load[$i] = number_format(($R_load[$i] / $R_unit[$_POST['result_unit']]), 2) ;
								
				echo "<th align=right>$R_load[$i]</th>";	//輸出測試電阻值
				
				$i ++;
				if ($i % $line == 0)
					echo "</tr>";
			}
			
			if ($i % $line != 0)
			{
				while ($i % $line != 0)
				{
					echo "<th bgcolor=cyan>&nbsp;</th>";
					echo "<th>&nbsp;</th>";
					$i ++;
				}
				echo "</tr>";
			}
				
			echo "</table>";
			echo "</th></tr>";
		}
		else if (isset($_POST['convert_type_csv']))	//選擇以CSV格式輸出結果
		{
			echo "<tr><th align=center>";
			echo "<table border=2>";
			echo "<tr><td>";
			echo "點位, 結果<br>";
			
			$i = 0;
			while ($i < $index)
			{
				$j = $i + 1;
				
				echo $j . ", ";
				
				//輸出電壓值換算
				if ($result[$i] + $ADC_offset > 4095)
					$Vout[$i] = 4095 * $Vss / 4096;
				else	$Vout[$i] = ($result[$i] + $ADC_offset) * ($Vss / 4096);
				//電阻值換算
				$R_load[$i] = ($Vout[$i] / ($Vss - $Vout[$i])) * $R_ref;
				$R_load[$i] = number_format(($R_load[$i] / $R_unit[$_POST['result_unit']]), 2);
			
				echo $R_load[$i];	//輸出測試電阻值
				
				echo "<br>";
				
				$i ++;
			}
					
			echo "</td></tr></table>";
			echo "</th></tr>";
		}
	}

?>
</form>
</table><br>


<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=blue>
<tr>
	<th><a href="./auto_test.php">返回上一頁</a></th>
</tr>
</table>
</center>

</body>
</html>