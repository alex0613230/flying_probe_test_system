# flying_probe_test_system
flying_probe_test_system 利用鋁擠型材料、步進馬達、3D 列印技術，和容易取得的嵌入式系統，建構 1 套相對便宜且適用於印刷電路板測試之飛針測試系統。其次，為提供系統監控功能，我們亦利用以網站為基礎之人機介面建立系統管理和即時狀態查詢系統。另外，為考量本系統應用於產業中的可行性，我們採用業界常見之與 Modbus 類似之通訊協定完成各設備間的資訊傳輸作業，以降低系統建置的成本，並提升設備的可擴充性。

本系統採用 Raspberry Pi 4 Model B 和 Espressif ESP32 兩種嵌入式系統。
Raspberry Pi 規劃為主控 (master) 設備，並在此硬體平台安裝 Raspberry Pi OS 作業系統、Apache Web Server，以及程式語言 PHP 和 C/C++ 等工具開發以網頁為基礎的使用者界面。
Espressif ESP32 規劃為受控 (slave) 設備的微控制器，以應用於控制飛針測試系統的探針移動與電路測試作業。

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

