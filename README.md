# flying_probe_test_system
我們所建構之飛針測試系統分包含連網設備、主控設備與受控設備等三種。

## 1.連網設備：<br>
	
 在連網設備與主控設備之間，本系統採用常見之無線網路 (Wi-Fi) 作為通訊管道，讓使用者隨時隨地可連線進行系統管理與檢測作業。
 另外，為方便使用者使用本系統，我們開發以網頁為基礎的使用者介面供使用者利用包含桌上型電腦、筆記型電腦、平板電腦與智慧型手機等可執行瀏覽程式 (browsers) 的連網設備提供容易使用的圖形化介面。

## 2.主控設備：<br>
　　我們利用 Raspberry Pi 硬體平台提供無線網路連線功能，並安裝網站伺服器 Apache與利用 HTML 與 PHP 程式語言開發網頁作為使用者介面。其次，為提供主控設備與受控設備之間的通訊功能，我們利用 USB 轉 UART 傳輸模組及訊號線作為通訊管道，並利用 C 語言完成兩者之間以修改後的 Modbus/ASCII 通訊協定進行資料傳輸作業。

## 3.受控設備：<BR>
　　我們採用 Espressif ESP32 作為處理器，負責控制探針移動機構與完成 PCB 測試作業。本研究採用 Arduino 應用整合開發環境 (integrated development environment, IDE) 完成軟體開發。為提供探針三軸定位功能，我們利用三顆步進馬達、螺絲紋桿、線性滑軌與相關感測模組設計探針移動機構，以提供可達 0.01mm 的水平定位精密度。在 PCB 測試電路部分，目前可提供單面單針式數位測試功能，以及單面雙針式類比測試功能。其中，數位測試功能可進行電路之短路與開路檢測，類比測試功能在搭配適當電路後可完成電阻值等相關檢測作業。

# 硬體電路區塊圖
<div align=center><img src ="https://github.com/alex0613230/flying_probe_test_system/blob/main/pic/circuit.png"/></div>
<br>

# 系統軟體架構
<div align=center><img src ="https://github.com/alex0613230/flying_probe_test_system/blob/main/pic/sys.png"/></div>
<br>

# 雛型圖
<div align=center><img src ="https://github.com/alex0613230/flying_probe_test_system/blob/main/pic/prototype.png"/></div>

# 使用者介面
<div align=center><img src ="https://github.com/alex0613230/flying_probe_test_system/blob/main/pic/index.png"/></div>
