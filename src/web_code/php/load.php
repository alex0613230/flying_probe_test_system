<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title>測試資料載入</title>
<style>
	a:hover {background-color:blue;color:white;}
	a {text-decoration:none;}
	.chi {font-family:"標楷體";font-size:24px;}
	td, th, button, input {font-size:24px;}
</style>
</head>
<body>

&nbsp;<br>
<center>
<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
<tr>
	<th height=30 colspan=2 bgcolor=yellow>
		<font size=6>測　試　資　料　載　入　功　能</font>
	</th>
</tr>
</table><p>

<?
	// 處理檔案
	$stdfile = $_FILES['stdfile']['tmp_name'];
	$data = file($stdfile);
	$co = count($data);
	
	// 開始處理檔案內容
	$pc = 0;	// 測試點計數器
	$index = 0;
	
	$XLZ = $YLZ = $ZLZ = $ZOF = $NOP = $PDT = 0;
	
	for ($i = 0; $i < $co; $i ++)
	{
		$data[$i] = trim($data[$i]);
		
		if (strlen($data[$i]) == 0)
			continue;
		
		if ($data[$i][0] == '#')
			continue;

		$ppp = preg_replace("/\s+/", " ", $data[$i]);
		$arr = explode(" ", $ppp);	// $arr[0]: keyword, $arr[1]...: value

		$kw = trim($arr[0]);

		if (strcmp($kw, "XLZ") == 0)		// X_logical_zero
		{
			$XLZ = $arr[1];
			
			$keyword[$index] = $kw;
			$values[$index][0] = $arr[1];
			$nov[$index] = 1;
			$index ++;
		}
		else if (strcmp($kw, "YLZ") == 0)	// Y_logical_zero
		{
			$YLZ = $arr[1];
			
			$keyword[$index] = $kw;
			$values[$index][0] = $arr[1];
			$nov[$index] = 1;
			$index ++;
		}
		else if (strcmp($kw, "ZLZ") == 0)	// Z_logical_zero
		{
			$ZLZ = $arr[1];
			
			$keyword[$index] = $kw;
			$values[$index][0] = $arr[1];
			$nov[$index] = 1;
			$index ++;
		}
		else if (strcmp($kw, "ZOF") == 0)	// Z_OFFSET
		{
			$ZOF = $arr[1];
			
			$keyword[$index] = $kw;
			$values[$index][0] = $arr[1];
			$nov[$index] = 1;
			$index ++;
		}
		else if (strcmp($kw, "NOP") == 0)	// Number of point
		{
			$NOP = $arr[1];
			
			$keyword[$index] = $kw;
			$values[$index][0] = $arr[1];
			$nov[$index] = 1;
			$index ++;
		}
		else if (strcmp($kw, "PDT") == 0)	// Probe delay time
		{
			$PDT = $arr[1];
			
			$keyword[$index] = $kw;
			$values[$index][0] = $arr[1];
			$nov[$index] = 1;
			$index ++;
		}
		else if (strcmp($kw, "PTR") == 0)	// PCB_test_data, x y function
		{
			$X_axis[$pc] = $arr[1];
			$Y_axis[$pc] = $arr[2];
			$Func[$pc] = $arr[3];
			
			$pc ++;
			
			$keyword[$index] = $kw;
			$values[$index][0] = $arr[1];
			$values[$index][1] = $arr[2];
			$values[$index][2] = $arr[3];
			$nov[$index] = 3;
			$index ++;
		}
	}	
		
	if ($pc != $NOP)
	{
		echo "<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>";
		echo "<tr>";
		echo "<th><font color=red>測試點數資料筆數不符，無法載入。</font></th>";
		echo "</tr>";
		echo "</table><p>";
	}
	else
	{
		echo "<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>";
		
		// 寫入參數資料至 ESP32
		$message = "0110002800060C";
		$message .= sprintf("%04X", $XLZ * 100);
		$message .= sprintf("%04X", $YLZ * 100);
		$message .= sprintf("%04X", $ZLZ * 100);
		$message .= sprintf("%04X", $ZOF * 100);
		$message .= sprintf("%04X", $NOP);
		$message .= sprintf("%04X", $PDT);
		
		$str = "./uart " . $message;
		$response = exec($str, $output, $ret);
		
		$error = 1;
		if (($response[2] == '1') && ($response[3] == '0'))
			$error = 0;
		
		if ($error == 1)
		{
			echo "<tr><th><font color=red>上傳參數資料失敗，ESP32 回應：" . $response . "</font></th></tr>";
		}
		else
		{
			echo "<tr><th><font color=blue>上傳參數資料成功</font></th></tr>";
			
			// 寫入測試點資料至 ESP32，每次 10 筆
			$START_ADDR = 51;
			$count_Q = floor($pc / 10);
			$count_R = $pc % 10;
			$times = 1;

			if ($count_Q > 0)
			{		
				for ($k = 0; $k < $count_Q; $k ++)
				{	
					$message = "0110";	
					$message .= sprintf("%04X", $START_ADDR);
					$message .= sprintf("%04X", 3 * 10);
					$message .= sprintf("%02X", 2 * 3 * 10);
					
					$pp = $k * 10;
					for ($m = 0; $m < 10; $m ++)
					{
						$ss = $pp + $m;
						$message .= sprintf("%04X", $X_axis[$ss] * 100);
						$message .= sprintf("%04X", $Y_axis[$ss] * 100);
						$message .= sprintf("%04X", $Func[$ss]);
					}
									
					$str = "./uart " . $message;
					$response = exec($str, $output, $ret);
					
					$error = 1;
					if (($response[2] == '1') && ($response[3] == '0'))
						$error = 0;
					
					if ($error == 1)
						break;
					
					$START_ADDR = $START_ADDR + 30;
				}
			}
			
			if ($error == 1)
			{
				echo "<tr><th><font color=red>上傳參數資料失敗，ESP32 回應：" . $response . "</font></th></tr>";
			}
			else
			{
				// 寫入測試點資料至 ESP32，剩餘筆數
				if ($count_R > 0)
				{
					$message = "0110";
					$message .= sprintf("%04X", $START_ADDR);		
					$message .= sprintf("%04X", 3 * $count_R);
					$message .= sprintf("%02X", 2 * 3 * $count_R);
					
					for ($m = $count_Q * 10; $m < $pc; $m ++)
					{
						$message .= sprintf("%04X", $X_axis[$m] * 100);
						$message .= sprintf("%04X", $Y_axis[$m] * 100);
						$message .= sprintf("%04X", $Func[$m]);
					}
					$str = "./uart " . $message;
					$response = exec($str, $output, $ret);
					
					$error = 1;
					if (($response[2] == '1') && ($response[3] == '0'))
						$error = 0;
				}
				
				if ($error == 1)
				{
					echo "<tr><th><font color=red>上傳測試點資料失敗，ESP32 回應：" . $response . "</font></th></tr>";
				}
				else
				{
					echo "<tr><th><font color=blue>上傳測試點資料成功，測試總點數：" . $NOP ."</font></th></tr>";
				}
			}
		}
		echo "</table><p>";
	}
?>

<table border=3 width=80% cellspacing=2 cellpadding=10 bordercolor=Blue>
<tr>
	<th><a href="./index.php">返回首頁</a></th>
</tr>
</table>
</center>
</form>
</body>
</html>
