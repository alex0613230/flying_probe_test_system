#include <esp_task_wdt.h>

#define  SW_VERSION    1
#define HW_VERSION    1
#define SLAVE_ID    1

#define PORTNO		5678	// local port no.

#define	TIMEOUT_DATA		2000
#define	SIZE		200
#define TRUE      1
#define FALSE     0

#define HR_SIZE     9000      // # of Holding registers
#define LINELIMIT   1000      // Modbus Message 的長度限制
#define TIMEOUT     1000
#define Total_Points    2000      // 可測試的總點數
#define RO_LIMIT    30      // RO 和 RW 暫存器的分隔點

// SLIM9-390 Setting=
#define mm_step_x   1       // 0.01 mm = 1 step T88 4細分 800step=8mm 1step=0.01mm
#define mm_step_y   1       // 0.01 mm = 1 step
#define mm_step_z   4       // 0.01 mm = 4 step T82 4細分 800step=2mm 4step=0.01mm
#define dir_Xforward   1
#define dir_Xbackward    0
#define dir_Yforward   	0
#define dir_Ybackward  	1  
#define dir_Zforward   0
#define dir_Zbackward    1
#define x_axis      0
#define y_axis      1
#define z_axis      2
#define X_LIMIT     17500    // X 軸上限, 17.5cm 2022/2/23
#define Y_LIMIT     13900     // Y 軸上限,13.9cm 2022/2/23
#define Z_LIMIT     3000      // Z 軸上限, 3cm	 2022/2/23
#define F_LIMIT     3     // 測試功能類型上限

// ESP32 Status
#define IDLE      0
#define MANUAL_OP   1
#define AUTO_OP     2
#define PAUSE     3
#define SMD_ALARM   4

// Register Allocation Table (Read only)
#define ADDR_SW_Version   0     // SW Version
#define ADDR_HW_Version   1     // HW Version

#define ADDR_Status     10      // ESP32 狀態
#define ADDR_Total_Point_1  11      // 測試總點數，值同 [ADDR_Cur_Point]
#define ADDR_Cur_Point_1  12      // 目前正被測試的點編號，1 ~ 2000
#define ADDR_Pass_Point   13      // 測試成功點數
#define ADDR_Complete_Point 14      // 測試完成點數，測試失敗點數 = 測試完成點數 - 測試成功點數
#define ADDR_X_Cur    15      // 目前 X 軸與原點的距離，單位為 0.01mm
#define ADDR_Y_Cur    16      // 目前 Y 軸與原點的距離，單位為 0.01mm
#define ADDR_Z_Cur    17      // 目前 Z 軸與原點的距離，單位為 0.01mm

// 供單步測試作業讀取用
#define ADDR_Total_Point_2  21      // 測試總點數，值同 [ADDR_Cur_Point]
#define ADDR_Cur_Point_2  22      // 目前正被測試的點編號，1 ~ 2000，值同 [ADDR_Cur_Point_1]，
#define ADDR_Single_Result  23      // 單步測試作業結果
#define ADDR_Single_Function  24      // 單步測試作業執行的功能
#define ADDR_Single_Status  25      // 單步測試作業執行狀態，0: idle，1: busy

// Register Allocation Table (Read/Write)
#define ADDR_OP_Code    31      // 運算碼
#define ADDR_OP_Data    32      // 與運算碼一起寫入的資料
#define ADDR_X_Zero   40      // PCB X 軸邏輯零點
#define ADDR_Y_Zero   41      // PCB Y 軸邏輯零點
#define ADDR_Z_Zero   42      // PCB Z 軸邏輯零點，探針接觸 PCB 的位置
#define ADDR_Z_Offset   43      // Z 軸上下位移量
#define ADDR_Total_Point  44      // 測試總點數:
#define ADDR_Probe_Delay  45      // 測試延遲時間
#define ADDR_LIMIT_1    45      // 值和最後一個位址相同，作為位址範圍判斷用

#define Point_Base    51      // 存放測試點資料的基底位址，每筆資料佔用三個暫存器，
              // 分別記錄 X 軸位置 (邏輯)、Y 軸位置 (邏輯) 和測試功能
              // 範圍：51 ~ 6050
          
#define Result_Base   7001      // 存放測試資料的基底位址，每個暫存器對應一個測試點
              // 範圍：7001 ~ 9000

// op_code
#define op_NOP      0     // No operation
#define op_Reset    1     // Reset ESP32，清除全部資料暫存器內容
#define op_Gohome   2     // X, Y, Z 軸歸位
#define op_Start    3     // 啟動自動測試作業
#define op_Pause    4     // 暫停自動測試作業
#define op_Continue   5     // 繼續自動測試作業
#define op_Stop     6     // 停止自動測試作業

#define op_Single_Cur   11      // 執行目前測試點手動單步測試作業
#define op_Single_Next    12      // 執行下第 N 測試點手動單步測試作業
#define op_Single_Pre   13      // 執行上第 N 測試點手動單步測試作業
#define op_Single_SCP   14      // 設定目前測試點編號

#define op_X_Forward    21      // 手動向前移動 X 軸 N * 0.01 mm 距離
#define op_X_BackWard   22      // 手動向後移動 X 軸 N * 0.01 mm 距離
#define op_Y_Forward    23      // 手動向前移動 Y 軸 N * 0.01 mm 距離
#define op_Y_BackWard   24      // 手動向後移動 Y 軸 N * 0.01 mm 距離
#define op_Z_Forward    25      // 手動向下移動 Z 軸 N * 0.01 mm 距離
#define op_Z_BackWard   26      // 手動向上移動 Z 軸 N * 0.01 mm 距離

// Exception Code for ModBus
#define MB_SUCCESS    00      // Message accepted success
#define MB_NOT_OWN    01      // Not message owner
#define MB_ILL_MSG    02      // illegal message
#define MB_ILL_Func   03      // illegal function
#define MB_ILL_Addr   04      // illegal data address
#define MB_ILL_Data   05      // illegal data value
#define MB_ILL_Busy   06      // device busy
#define MB_ILL_OPC    07      // illegal operation code

// 測試功能
#define Short_ckt_G   0     // GND 短路測試，DI
#define Short_ckt_V   1     // Vss 短路測試，DI
#define Analog_test   2     // 類比測試，AI


typedef word	WORD;
typedef	byte	BYTE;

typedef struct
{
	WORD	transaction_id;
	WORD	protocol_id;
	WORD	length;		// bytes of unit_id + function_code + ...
	BYTE	unit_id;
	BYTE	function_code;
	WORD	starting_address;
	WORD	no_of_points;
	BYTE	byte_count;
	WORD	reg_values[HR_SIZE];
} MOD_PACKET;

typedef struct
{
	WORD	fd_value;
	WORD	remote_port;
	char	remote_address[50];
	WORD	length;				// 必須是 4 的倍數
	char	data[LINELIMIT];
} UDP_PACKET;

int		udp_index;			// 執行 AT+USOCKREG 產生的 udp socket index
UDP_PACKET	recv_udp;
MOD_PACKET	recv_modbus;



WORD		input_status, input_index;
unsigned long	input_timer;

// Memory space for holding registers
WORD    hold_registers[HR_SIZE];

// Variables used for reading input recv_modbus
BYTE    status, indexpp;
BYTE    input_buf[LINELIMIT], output_buf[LINELIMIT];
BYTE    binary_buf[LINELIMIT/2];
unsigned long timer;
BYTE    command_flag;

// Core variables
TaskHandle_t  Task1;
int   task_Single_distance;
BYTE    task_axis, task_dir;
int     task_step;
bool    Motor_Reset_Flag = false;
bool    Motor_Flag = false;
bool    Auto_Test_Flag = false;
bool    Pause_Flag = false;
bool    Stop_Flag = false;
bool    Single_Test_Flag = false;

// ESP32 接腳設定
BYTE    PUL[3]  = {26, 15, 21};   // 步進馬達 Pulse 腳位，DO
BYTE    DIR[3]  = {25, 14, 19};   // 步進馬達 DIR 腳位，DO
//BYTE    ALM[3]  = {, , };   // 步進馬達 ALARM 腳位，DI
BYTE    HOME[3] = {32, 35, 34};   // 機台 HOME

//腳位，DI
BYTE    en[3]    = {33, 27, 18};
BYTE    Probe_DI = 4;     // DI 探針腳位
BYTE    Probe_AI = 33;      // AI 探針腳位

//////////////////////////////////////////////////////////////////////////

// utility
BYTE to_binary(BYTE firstbyte, BYTE secondbyte)         // Core 1
{
  BYTE  po = 0;

  if (firstbyte <= '9')
    po = (firstbyte - '0');
  else  po = (firstbyte - 'A') + 10;

  po = po << 4;
    
  if (secondbyte <= '9')
    po |= (secondbyte - '0');
  else  po |= (secondbyte - 'A') + 10;

  return po;
}

// utility
void to_hexascii(BYTE tmp[2], BYTE data)          // Core 1
 {
  BYTE  p;
  
  p = (data & 0XF0) >> 4;
  if (p <= 9)
    tmp[0] = '0' + p;
  else  tmp[0] = 'A' + p - 10;
  
  p = data & 0X0F;
  if (p <= 9)
    tmp[1] = '0' + p;
  else  tmp[1] = 'A' + p - 10;
}

void send_response_packet(BYTE *job, WORD length, UDP_PACKET *udp)
{
	char	output[LINELIMIT], ttt[10];
	BYTE	tmp[2];
	
	char	data[LINELIMIT];
	int	i, len;
	
	len = 0;
	for (i = 0; i < length; i ++)
	{
		to_hexascii(tmp, job[i]);
		data[len ++] = char(tmp[0]);
		data[len ++] = char(tmp[1]);
	}
	
	while (len % 4 != 0)
	{
		data[len] = '0';
		len ++;
	}
	data[len] = '\0';
	
	sprintf(output, "AT+USOCKSEND=%d,%d,\"%s\",%d,\"%s\"",
			udp_index,
			udp->remote_port,
			udp->remote_address,
			len,
			data);
	
	/*
	strcpy(output, "AT+USOCKSEND=");
	itoa(udp_index, ttt, 10);
	strcat(output, ttt);
	strcat(output, ",");
	
	itoa(udp->remote_port, ttt, 10);
	strcat(output, ttt);
	strcat(output, ",\"");
	
	strcat(output, udp->remote_address);
	strcat(output, "\",");
	
	itoa(len, ttt, 10);
	strcat(output, ttt);
	
	strcat(output, ",\"");
	strcat(output, data);
	strcat(output, "\"");
	*/
	
	Serial.println(output);
	
	while (! Serial.available()) ;		// 等候 VC7300 回應

	while (! Serial.find("OK"))
	{
		Serial.println(output);
		delay(100);
	}
}

void gen_exception_response_packet(BYTE *msg, WORD *size, BYTE except_code, MOD_PACKET *input)
{
	BYTE	j = 0;

	msg[j ++] = input->transaction_id / 256;
	msg[j ++] = input->transaction_id % 256;
	msg[j ++] = input->protocol_id / 256;
	msg[j ++] = input->protocol_id % 256;
	msg[j ++] = input->length / 256;
	msg[j ++] = input->length % 256;
	msg[j ++] = input->unit_id;
	msg[j ++] = input->function_code | 0X80;
	msg[j ++] = except_code;
	*size = j;
}

// for 0X03: read Holding Registers
void gen_03_response_packet(BYTE *msg, WORD *size, MOD_PACKET *input)
{
	BYTE	i, j;
	BYTE	bytecount = 0;
	WORD	data;
	
	if (input->starting_address + input->no_of_points > SIZE)
	{
		gen_exception_response_packet(msg, size, 0X02, input);
		return;
	}
	
	j = 0;
	
	msg[j ++] = input->transaction_id / 256;
	msg[j ++] = input->transaction_id % 256;
	msg[j ++] = input->protocol_id / 256;
	msg[j ++] = input->protocol_id % 256;
	
	data = 3 + input->no_of_points * 2;	// for length
	msg[j ++] = data / 256;
	msg[j ++] = data % 256;
	
	msg[j ++] = input->unit_id;
	
	if (input->unit_id != SLAVE_ID)
	{
		gen_exception_response_packet(msg, size, MB_NOT_OWN, input);
		return;
	}
	
	msg[j ++] = input->function_code;
	
	bytecount = input->no_of_points * 2;
	msg[j ++] = (BYTE) (bytecount);
	
	for (i = 0; i < input->no_of_points; i ++)
	{
		data = hold_registers[input->starting_address + i];
		msg[j ++] = data / 256;
		msg[j ++] = data % 256;
	}
	
	*size = j;
}

// for 0X10: Preset Multiple Registers
void gen_10_response_packet(BYTE *msg, WORD *size, MOD_PACKET *input)
{
	BYTE	i, j;
	BYTE	bytecount = 0;
	WORD	data;
	
	if (input->starting_address + input->no_of_points > SIZE)
	{
		gen_exception_response_packet(msg, size, 0X02, input);
		return;
	}
	
	j = 0;
	
	msg[j ++] = input->transaction_id / 256;
	msg[j ++] = input->transaction_id % 256;
	msg[j ++] = input->protocol_id / 256;
	msg[j ++] = input->protocol_id % 256;
	
	data = 6;
	msg[j ++] = data / 256;		// for length
	msg[j ++] = data % 256;
	msg[j ++] = input->unit_id;
/*
	if (input->unit_id != SLAVE_ID)
	{
		gen_exception_response_packet(msg, size, MB_NOT_OWN, input);
		return;
	}
*/
	msg[j ++] = input->function_code;
	
	msg[j ++] = (BYTE) (input->starting_address / 256);
	msg[j ++] = (BYTE) (input->starting_address % 256);
	
	msg[j ++] = (BYTE) (input->no_of_points / 256);
	msg[j ++] = (BYTE) (input->no_of_points % 256);
	
	*size = j;
}

void read_input(void)
{
	char	ch;
	
	ch = Serial.read();
	
	//Serial.print(ch);
	
	switch(input_status)
	{
		case 0:	if (ch == '+')	// leading character
			{
				input_status = 1;
				input_index = 0;
				
				input_buf[input_index] = ch;
				input_index ++;
				
				input_timer = millis();
			}
			break;
			
		case 1: if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')
				|| (ch == '\"')	|| (ch == ',') || (ch == ' ') || (ch == ':'))
			{
				if (ch >= 'a' && ch <= 'f')
					ch = ch - 32;
				
				input_status = 1;
				input_buf[input_index] = ch;
				input_index ++;
				
				if (input_index >= LINELIMIT)	// input buffer overrun
				{
					input_status = 0;
					input_index = 0;
				}
				
				input_timer = millis();
			}
			else if (ch == '\n')
			{
				input_status = 2;
				input_buf[input_index] = '\0';
				input_timer = millis();
			}
			break;
	}
}


// 檢查 RPi 輸入資料並轉換成 recv_modbus 結構
int parse_input(char *input, UDP_PACKET *udp, MOD_PACKET *mod)
{
	char	*p, ttt[100];
	int	i, j;
	BYTE	data;
	WORD	wt;
	
	p = input;
	
	if (*p == '\0')
		return MB_ILL_MSG;
	
	i = 0;			// 抓取 "+USOCKRECV:"
	while (*p != ' ')
	{
		ttt[i] = *p;
		p ++;
		i ++;
		if (*p == '\0')
			return MB_ILL_MSG;
	}
	ttt[i] = '\0';
	p ++;
	
	//if (strcmp("+USOCKRECV:", ttt) != 0)
		//return FALSE;
	
	p ++;	// skip ' '
	
	i = 0;			// 抓取 fd_value
	while (*p != ',')
	{
		ttt[i] = *p;
		
		i ++;
		p ++;
		if (*p == '\0')
			return MB_ILL_MSG;
	}
	ttt[i] = '\0';
	p ++;
	
	udp->fd_value = atoi(ttt);
	
	i = 0;			// 抓取 remote_port
	while (*p != ',')
	{
		ttt[i] = *p;
		
		i ++;
		p ++;
		if (*p == '\0')
			return MB_ILL_MSG;
	}
	ttt[i] = '\0';
	p ++;
	
	udp->remote_port = atoi(ttt);
	
	while (*p != '\"')
	{
		p ++;
		if (*p == '\0')
			return MB_ILL_MSG;
	}
	p ++;
	
	i = 0;			// 抓取 remote_address
	while (*p != '\"')
	{
		ttt[i] = *p;
		
		p ++;
		i ++;
		if (*p == '\0')
			return MB_ILL_MSG;
	}
	ttt[i] = '\0';
	p ++;
	
	strcpy(udp->remote_address, ttt);
	
	while (*p != ',')	// 跳過 ','
	{
		p ++;
		if (*p == '\0')
			return MB_ILL_MSG;
	}
	p ++;
	
	i = 0;			// 抓取 length
	while (*p != ',')
	{
		ttt[i] = *p;
		
		i ++;
		p ++;
		if (*p == '\0')
			return MB_ILL_MSG;
	}
	ttt[i] = '\0';
	p ++;
	udp->length = atoi(ttt);
	
	while (*p != '\"')	// 尋找下一個 "
	{
		p ++;
		if (*p == '\0')
			return MB_ILL_MSG;
	}
	p ++;
	
	i = 0;		// 抓取 data
	while (*p != '\"')
	{
		ttt[i] = *p;
		
		p ++;
		i ++;
		if (*p == '\0')
			return MB_ILL_MSG;
	}
	ttt[i] = '\0';
	p ++;
	
	strcpy(udp->data, ttt);
	
	// parse MODBUS
	if (i < 10)
		return MB_ILL_MSG;
	
	p = udp->data;
	
	wt = to_binary(*p, *(p+1));
	p = p + 2;
	wt = wt * 256 + to_binary(*p, *(p+1));
	p = p + 2;
	mod->transaction_id = wt;
	
	wt = to_binary(*p, *(p+1));
	p = p + 2;
	wt = wt * 256 + to_binary(*p, *(p+1));
	p = p + 2;
	mod->protocol_id = wt;
	
	wt = to_binary(*p, *(p+1));
	p = p + 2;
	wt = wt * 256 + to_binary(*p, *(p+1));
	p = p + 2;
	mod->length = wt;
	
	mod->unit_id = to_binary(*p, *(p+1));
	p = p + 2;
	
	mod->function_code = to_binary(*p, *(p+1));
	p = p + 2;
	
	wt = to_binary(*p, *(p+1));
	p = p + 2;
	wt = wt * 256 + to_binary(*p, *(p+1));
	p = p + 2;
	mod->starting_address = wt;
	
	wt = to_binary(*p, *(p+1));
	p = p + 2;
	wt = wt * 256 + to_binary(*p, *(p+1));
	p = p + 2;
	mod->no_of_points = wt;
	
	switch (mod->function_code)
	{
		case 0X03: break;
		
		case 0X10:
			// get byte count, 1 bytes
			mod->byte_count = to_binary(*p, *(p+1));
			p = p + 2;
			
			if (mod->byte_count <= 0)
				return MB_ILL_MSG;
			
			if (i - 24 < mod->byte_count)
				return MB_ILL_MSG;
			
			// get registers' values in WORD
			for (i = 0; i < mod->no_of_points; i++)
			{
				wt = to_binary(*p, *(p+1));
				p = p + 2;
				wt = wt * 256 + to_binary(*p, *(p+1));
				p = p + 2;
				mod->reg_values[i] = wt;
			}
			
			break;
		default:
			return MB_ILL_Func;
	}
	
	return MB_SUCCESS;
}

// 檢查資料內容
WORD data_check(MOD_PACKET *fr)              // Core 1
{
  WORD  i, index, data, pp;
  WORD  DI_flag = 0, AI_flag = 0;
  
  for (i = 0; i < fr->no_of_points; i ++)
  {
    index = fr->starting_address + i;
    data  = fr->reg_values[i];
    
    if (index <= ADDR_LIMIT_1)
    {
      switch (index)
      {
        case ADDR_OP_Code:
          if (data > 26)
            return MB_ILL_OPC;
          
          if ((data > op_Stop) && (data < op_Single_Cur))
            return MB_ILL_OPC;
          
          if ((data > op_Single_SCP) && (data < op_X_Forward))
            return MB_ILL_OPC;
          
          if ((data >= 11) && (data <= 14)) // 屬於單步測試作業
          {
            if (hold_registers[ADDR_Total_Point_2] == 0)
              return MB_ILL_OPC;
          }
          
          // 若處於自動測試作業中，則只能輸入 Pause, Continue, 和 Stop 命令
          if (hold_registers[ADDR_Status] == AUTO_OP)
          {
            if ((data < op_Pause) || (data > op_Stop))
              return MB_ILL_OPC;
          }
          
          // 若處於暫停自動測試作業中，則只能輸入 Continue 和 Stop 命令
          if (hold_registers[ADDR_Status] == PAUSE)
          {
            if ((data < op_Continue) || (data > op_Stop))
              return MB_ILL_OPC;
          }
          
          break;
          
        case ADDR_OP_Data: // 只檢查最大範圍, X_LIMIT, Y_LIMIT
          if (data > X_LIMIT)
            return MB_ILL_Data;
          
          if (data > Y_LIMIT)
            return MB_ILL_Data;
          
          break;
          
        case ADDR_X_Zero:
          if (data > X_LIMIT)
            return MB_ILL_Data;
          
          break;
          
        case ADDR_Y_Zero:
          if (data > Y_LIMIT)
            return MB_ILL_Data;
          
          break;
          
        case ADDR_Z_Zero:
          if (data > Z_LIMIT)
            return MB_ILL_Data;
          
          break;
          
        case ADDR_Z_Offset:
          if (data > Z_LIMIT)
            return MB_ILL_Data;
          
          break;
            
        case ADDR_Total_Point:
          if (data > Total_Points)
            return MB_ILL_Data;
          
          break;
          
        case ADDR_Probe_Delay:
          break;
          
        default:  return MB_ILL_Addr;
      }
    }
    else  // 檢查位址 51 ~ 6050 的測試點資料
    {
      pp = (index - 51) % 3;
      
      switch (pp)
      {
        case 0: // X 軸資料
          if (data + hold_registers[ADDR_X_Zero] > X_LIMIT)
            return MB_ILL_Data;
          break;
          
        case 1: // Y 軸資料
          if (data + hold_registers[ADDR_Y_Zero] > Y_LIMIT)
            return MB_ILL_Data;
          break;
        
        case 2: // 測試功能資料
          if (data >= F_LIMIT)
            return MB_ILL_Data;
          
          if (data < 2)
            DI_flag = 1;
          else if (data == 2)
            AI_flag = 1;
          
          break;
      }
    }
  }
  
  // DI 和 AI 測試不能同時存在
  if ((DI_flag == 1) && (AI_flag == 1))
    return MB_ILL_Data;
  
  return MB_SUCCESS;
} 

// 初始化設定
void Initial_set(void)                // Core 1
{
  BYTE  i;
  
  Serial.begin(115200);
  Serial2.begin(115200);
  
  // 設定 ESP32 接腳屬性
  for (i = 0; i < 3; i ++)
  {
    pinMode(PUL[i], OUTPUT);
    pinMode(DIR[i], OUTPUT);
//    pinMode(ALM[i], INPUT_PULLUP);
    pinMode(HOME[i], INPUT);
  }

  // 在核心 0 啟動 Task1
  xTaskCreatePinnedToCore
  (
    Task1_senddata,
    "Task1",
    10000,
    NULL,
    0,
    &Task1,
    0
  ); 
  
  esp_task_wdt_init(10, false);
}

void Initial_WI_SUN()
{
	char	ss[LINELIMIT], ttt[50], ppp[50];
	int	i, j;
	delay(3000);
	
	Serial2.println();
	Serial2.println();
	
	strcpy(ss, "uart ATCMD");
	Serial2.println(ss);
	delay(1000);
	
	strcpy(ss, "AT+ECHO=0");
	Serial.println(ss);
	
	delay(1000);
	
	strcpy(ss, "AT+USOCKREG");		// udp_index
	Serial.println(ss);
	
	udp_index = 0;
	
	delay(1000);
	
	sprintf(ss, "%s%d,%d", "AT+USOCKBIND=", udp_index, PORTNO);
	Serial.println(ss);
}

void setup()                  // Core 1
{ 
  // Initial Holding Registers
  Initial_HR();
  Initial_set();
  Initial_WI_SUN();
  // Open serial communications

  
  // Initial global variables
  status = 0;
  indexpp = 0;
  command_flag = 0;
  
  // 執行三軸歸位
  Motor_Reset_Flag = true;
  hold_registers[ADDR_X_Cur] = 0;
  hold_registers[ADDR_Y_Cur] = 0;
  hold_registers[ADDR_Z_Cur] = 0;
}
void Wi_SUN_RECEIVE()
{
	WORD	length, i;
	
	if (Serial.available())
		read_input();
	// input timeout, reset input buffer indexpp and status
	if (input_status != 0)
	{
		if (millis() - input_timer > TIMEOUT_DATA)
		{
			input_index = 0;
			input_status = 0;
			input_timer = millis();
		}
	}
}

void loop()                 // Core 1
{
  WORD  length, i, rr, cc;
  int address1, address2;
  
  // 檢查三軸 ALARM 訊號
  digitalWrite(en[0], LOW);
  digitalWrite(en[1], LOW);
  digitalWrite(en[2], LOW);
  
  // 讀取 RPi 輸入資料
  if(Serial.available())
	Wi_SUN_RECEIVE();
  
  // 處理 RPi 輸入資料
  if (status == 2)
  {
    Serial.print("Input > ");
    for (i = 0; i < indexpp; i ++)
      Serial.print(char(input_buf[i]));
    Serial.println();
    
    rr = parse_input();
    switch (rr)
    {
      case MB_SUCCESS: // Success
        if (recv_modbus.function == 0X03)
        {
          address1 = (int) recv_modbus.start_address;
          address2 = (int) recv_modbus.start_address + (int) recv_modbus.no_of_points;
          
          if ((address1 >= HR_SIZE) || (address2 >= HR_SIZE))
            gen_exception_response_frame(output_buf, &length, MB_ILL_Addr);
          else  gen_03_response_frame(output_buf, &length);
          
          send_response_frame(output_buf, length);
          send_response_frame_debug(output_buf, length);
        }
        else if (recv_modbus.function == 0X10)
        {
          address1 = (int) recv_modbus.start_address;
          address2 = (int) recv_modbus.start_address + (int) recv_modbus.no_of_points;
          
          if ((address1 <= RO_LIMIT) || (address2 >= Result_Base))
          {
            gen_exception_response_frame(output_buf, &length, MB_ILL_Addr);
          }
          else if (hold_registers[ADDR_Status] == IDLE)
          {
            cc = data_check(&recv_modbus);
            
            if (cc == 0)
            {
              for (i = 0; i < recv_modbus.no_of_points; i ++)
              {
                hold_registers[recv_modbus.start_address + i] = recv_modbus.reg_values[i];
                
                if ((recv_modbus.start_address + i) == ADDR_Total_Point)
                {
                  hold_registers[ADDR_Total_Point_1] = recv_modbus.reg_values[i];
                  hold_registers[ADDR_Total_Point_2] = recv_modbus.reg_values[i];
                }
              }
            
              if ((address1 == ADDR_OP_Code) && (recv_modbus.no_of_points <= 2))
                command_flag = 1;
            
              gen_10_response_frame(output_buf, &length);
            }
            else  gen_exception_response_frame(output_buf, &length, cc);
          }
          else if ((hold_registers[ADDR_Status] == MANUAL_OP)
            || (hold_registers[ADDR_Status] == AUTO_OP)
            || (hold_registers[ADDR_Status] == PAUSE)
            || (hold_registers[ADDR_Status] == SMD_ALARM))
          {
            if ((address1 == ADDR_OP_Code) && (recv_modbus.no_of_points <= 2))
            {
              cc = data_check(&recv_modbus);
            
              if (cc == 0)
              {
                for (i = 0; i < recv_modbus.no_of_points; i ++)
                  hold_registers[recv_modbus.start_address + i] = recv_modbus.reg_values[i];
                
                gen_10_response_frame(output_buf, &length);
                
                command_flag = 1;
              }
              else  gen_exception_response_frame(output_buf, &length, cc);
            }
            else  gen_exception_response_frame(output_buf, &length, MB_ILL_Busy);
          }
          
          send_response_frame(output_buf, length);
          send_response_frame_debug(output_buf, length);  
        }
        break;
        
      case MB_NOT_OWN:
        break;
      
      case MB_ILL_MSG:
        gen_exception_response_frame(output_buf, &length, MB_ILL_MSG);
        send_response_frame(output_buf, length);
        send_response_frame_debug(output_buf, length);
        break;
        
      case MB_ILL_Func:
        gen_exception_response_frame(output_buf, &length, MB_ILL_Func);
        send_response_frame(output_buf, length);
        send_response_frame_debug(output_buf, length);
        break;
    }
    
    indexpp = 0;
    status = 0;
  }
  
  // 判斷 RPi 輸入是否逾時
  if (status != 0)
  {
    if (millis() - timer > TIMEOUT)
    {
      indexpp = 0;
      status = 0;
      timer = millis();
    }
  }
  
  // 若有命令進來，則執行命令的作業
  if (command_flag == 1)
  {
    do_command(); 
    command_flag = 0;
  }
  else        // 否則依目前狀態持續進行
  {
    if (hold_registers[ADDR_Status] == AUTO_OP) // 執行自動測試
    {
      if (Auto_Test_Flag == false)
      {
        do_auto_op();
      }
    }
  }
}

// 執行命令
void do_command(void)               // Core 1
{
  switch (hold_registers[ADDR_OP_Code])
  {
    case op_NOP:         break;
    case op_Reset:    Initial_HR();    break;   // 重設 ESP32，清除暫存器內容
    case op_Gohome:   Gohome();  break;   // 三軸歸位
    
    // 自動測試作業四項作業
    case op_Start:    Auto_Start();    break;
    case op_Pause:    Auto_Pause();    break;
    case op_Continue:   Auto_Continue(); break;
    case op_Stop:     Auto_Stop();   break;
    
    // 手動測試作業四項作業
    case op_Single_Cur:   Single_OP(0);    break;
    case op_Single_Next:  Single_OP(hold_registers[ADDR_OP_Data]);  break;
    case op_Single_Pre:   Single_OP(- hold_registers[ADDR_OP_Data]);  break;
    
    case op_Single_SCP: // 設定目前測試點位
      if (hold_registers[ADDR_OP_Data] > hold_registers[ADDR_Total_Point_2])
      {
        hold_registers[ADDR_Cur_Point_2] = hold_registers[ADDR_Total_Point_2];
        hold_registers[ADDR_Cur_Point_1] = hold_registers[ADDR_Total_Point_2];
      }
      else if (hold_registers[ADDR_OP_Data] == 0)
      {
        hold_registers[ADDR_Cur_Point_2] = 1;
        hold_registers[ADDR_Cur_Point_1] = 1;
      }
      else
      {
        hold_registers[ADDR_Cur_Point_2] = hold_registers[ADDR_OP_Data];
        hold_registers[ADDR_Cur_Point_1] = hold_registers[ADDR_OP_Data];
      }
      
      break;
    
    // 手動移動三軸
    case op_X_Forward:  X_move(hold_registers[ADDR_OP_Data]);   break;
    case op_X_BackWard:   X_move(- hold_registers[ADDR_OP_Data]); break;
    case op_Y_Forward:  Y_move(hold_registers[ADDR_OP_Data]);   break;
    case op_Y_BackWard:   Y_move(- hold_registers[ADDR_OP_Data]); break;
    case op_Z_Forward:  Z_move(hold_registers[ADDR_OP_Data]); break;
    case op_Z_BackWard:   Z_move(- hold_registers[ADDR_OP_Data]); break;
  }
}

// 執行自動測試作業
void do_auto_op(void)               // Core 1
{
  int cp;
  
  if (hold_registers[ADDR_Status] != AUTO_OP)
    return;
  
  cp = hold_registers[ADDR_Cur_Point_1] + 1;
  
  if (cp > hold_registers[ADDR_Total_Point])  // Complete
  {
    hold_registers[ADDR_Status] = IDLE;
    Gohome();
    return;
  }

  Auto_Test_Flag = true;
}

// 重設 ESP32，清除暫存器內容
void Initial_HR(void)               // Core 1
{
  for (int i = 0; i < HR_SIZE; i ++)
    hold_registers[i] = 0;
  
  Motor_Reset_Flag = true;    // 執行三軸歸位
  
  hold_registers[ADDR_SW_Version] = SW_VERSION;
  hold_registers[ADDR_HW_Version] = HW_VERSION;
  
  hold_registers[ADDR_Status] = IDLE;
}

// 三軸歸位
void Gohome(void)               // Core 1
{
  Motor_Reset_Flag = true;
  
  hold_registers[ADDR_X_Cur] = 0;
  hold_registers[ADDR_Y_Cur] = 0;
  hold_registers[ADDR_Z_Cur] = 0;
}

// 自動測試作業開始
void Auto_Start(void)               // Core 1
{
  WORD  i, total;
  
  hold_registers[ADDR_Status] = AUTO_OP;
  hold_registers[ADDR_Cur_Point_1] = hold_registers[ADDR_Cur_Point_2] = 0;
  hold_registers[ADDR_Pass_Point] = 0;
  hold_registers[ADDR_Complete_Point] = 0;
  
  total  = Result_Base + Total_Points;
  for (i = Result_Base; i < total; i ++)
    hold_registers[i] = 0;
}

// 自動測試作業暫停
void Auto_Pause(void)               // Core 1
{
  hold_registers[ADDR_Status] = PAUSE;
  Pause_Flag = true;
}

// 自動測試作業繼續
void Auto_Continue(void)              // Core 1
{
  hold_registers[ADDR_Status] = AUTO_OP;
  Pause_Flag = false;
}

// 自動測試作業停止
void Auto_Stop(void)                // Core 1
{
  hold_registers[ADDR_Status] = IDLE;
  Stop_Flag = true;
}

// 手動測試作業
void Single_OP(int distance)              // Core 1
{
  task_Single_distance = distance;
  Single_Test_Flag = true;
  
}

// 移動 X 軸
void X_move(int distance)               // Core 1
{
  int dd;

  dd = (int) hold_registers[ADDR_X_Cur] + distance; 
  
  if (dd > X_LIMIT)
  {
    distance = X_LIMIT - hold_registers[ADDR_X_Cur];
    motor_move(x_axis, distance, dir_Xforward);
    dd = X_LIMIT;
  }
  else if (dd < 0)
  { 
    distance = (int) hold_registers[ADDR_X_Cur];
    motor_move(x_axis, distance, dir_Xbackward);
    dd = 0;
  }
  else if ((int) hold_registers[ADDR_X_Cur] > dd) 
  {
    motor_move(x_axis, -distance, dir_Xbackward);
  }
  else if ((int) hold_registers[ADDR_X_Cur] < dd) 
  {
    motor_move(x_axis, distance, dir_Xforward);
  }
  
  hold_registers[ADDR_X_Cur] = (WORD) dd;
  hold_registers[ADDR_Status] = IDLE;
}

// 移動 Y 軸
void Y_move(int distance)             // Core 1
{
  int dd;
  
  dd = (int) hold_registers[ADDR_Y_Cur] + distance;
  
  if (dd > Y_LIMIT)
  {
    distance = Y_LIMIT - hold_registers[ADDR_Y_Cur];
    motor_move(y_axis, distance, dir_Yforward);
    dd = Y_LIMIT;
  }
  else if (dd < 0)
  { 
    distance = (int) hold_registers[ADDR_Y_Cur];
    motor_move(y_axis, distance, dir_Ybackward);
    dd = 0;
  }
  else if ((int) hold_registers[ADDR_Y_Cur] > dd) 
  {
    motor_move(y_axis, -distance, dir_Ybackward);
  }
  else if ((int) hold_registers[ADDR_Y_Cur] < dd) 
  {
    motor_move(y_axis, distance, dir_Yforward);
  }
  
  hold_registers[ADDR_Y_Cur] = (WORD) dd;
  hold_registers[ADDR_Status] = IDLE;
}

// 移動 Z 軸
void Z_move(int distance)             // Core 1
{
  int dd;
  
  dd = (int) hold_registers[ADDR_Z_Cur] + distance;
  
  if (dd > Z_LIMIT)
  {
    distance = Z_LIMIT - hold_registers[ADDR_Z_Cur];
    motor_move(z_axis, distance, dir_Zbackward);
    dd = Z_LIMIT;
  }
  else if (dd < 0)
  { 
    distance = (int) hold_registers[ADDR_Z_Cur];
    motor_move(z_axis, distance, dir_Zforward);
    dd = 0;
  }
  else if ((int) hold_registers[ADDR_Z_Cur] > dd) 
  {
    motor_move(z_axis, -distance, dir_Zbackward);
  }
  else if ((int) hold_registers[ADDR_Z_Cur] < dd) 
  {
    motor_move(z_axis, distance, dir_Zforward);
  }
  
  hold_registers[ADDR_Z_Cur] = (WORD) dd;
  hold_registers[ADDR_Status] = IDLE;
}

// 通知 Core 0 作業
void motor_move(BYTE c_axis, int mm_move, BYTE dir_move)      // Core 1
{
  task_axis = c_axis;
  task_step = mm_move;
  task_dir = dir_move;
  Motor_Flag = true;
}

// Core 0 主作業
void Task1_senddata(void * pvParameters)          // Core 0
{
  for (;;) 
  {
    if (Stop_Flag == true)        // 停止自動測試作業過程
    {
      Single_Test_Flag = false;
      Auto_Test_Flag = false;
      Pause_Flag = false;
      Motor_Reset_Flag = false;
      Motor_Flag = false;
      
      Gohome(); // 三軸歸位
      
      Stop_Flag = false;
    }
    else if (Single_Test_Flag)      // 手動測試作業
    {
      Single_Point_Test(task_Single_distance);
      Single_Test_Flag = false;
    } 
    else if ((Auto_Test_Flag) && (! Pause_Flag))  // 自動測試作業，每次完成一個測試點位
    {
      Auto_Test();
      Auto_Test_Flag = false;
    }
    else if (Motor_Reset_Flag)      // 三軸歸位
    {
      Motor_Home(); 
      Motor_Reset_Flag = false;
    }
    else if (Motor_Flag)        // 手動移動步進馬達
    {
      Auto_Motor(task_axis, task_step, task_dir);
      Motor_Flag = false;
    }
    else
    {
      delay(1); //Task1休息，delay(1)不可省略
    }
  }
}

// 手動測試作業
void Single_Point_Test(int distance)            // Core 0
{
  int cp, x, y, f;
  
  hold_registers[ADDR_Single_Status] = 1;   
  
  // 計算目前測試點位
  cp = hold_registers[ADDR_Cur_Point_1] + distance;
  if (cp > hold_registers[ADDR_Total_Point])
    cp = hold_registers[ADDR_Total_Point];
  else if (cp < 1)
    cp = 1;
  
  hold_registers[ADDR_Cur_Point_1] = cp;
  hold_registers[ADDR_Cur_Point_2] = hold_registers[ADDR_Cur_Point_1];
  
  // 讀取 x, y, function 資料
  x  = hold_registers[Point_Base + (cp - 1) * 3];
  y  = hold_registers[Point_Base + (cp - 1) * 3 + 1];
  f  = hold_registers[Point_Base + (cp - 1) * 3 + 2];
  
  hold_registers[ADDR_Single_Function] = f;
  
  // 移動 X, Y 軸
  Auto_Coordinate_set_xy(x, y);
  
  // 下針
  Auto_Coordinate_set_z(hold_registers[ADDR_Z_Zero]);
  
  // 測試
  hold_registers[ADDR_Single_Result] = Prob_function(f);
    
  // 上針
  Auto_Coordinate_set_z(hold_registers[ADDR_Z_Offset]);

  hold_registers[ADDR_Status] = IDLE;
  hold_registers[ADDR_Single_Status] = IDLE;
}

// 自動測試作業
void Auto_Test(void)                // Core 0
{
  WORD    result, f, x, y, p_index, f_index;
  int   cp;
  
  cp = hold_registers[ADDR_Cur_Point_1] + 1;
      
  p_index = Point_Base + (cp - 1) * 3;  // 該測試點資料的暫存器位址
  x = hold_registers[p_index];    // 該點 X 軸的值
  y = hold_registers[p_index + 1];  // 該點 Y 軸的值
  f = hold_registers[p_index + 2];  // 該點的測試功能
  
  // 移動 X, Y 軸
  Auto_Coordinate_set_xy(x, y);
  
  // 下針
  Auto_Coordinate_set_z(hold_registers[ADDR_Z_Zero]);
  
  // 測試
  result = Prob_function(f);  // 測試結果，0: 成功，1: 失敗，其他 AI 值
  
  // 上針
  Auto_Coordinate_set_z(hold_registers[ADDR_Z_Offset]);
  
  // 記錄測試結果
  f_index = Result_Base + (cp - 1);
  hold_registers[f_index] = result;
  
  // 更新其他暫存器內容
  if (f == 2)
    hold_registers[ADDR_Pass_Point] ++;
  else if (f < 2)
  {
    if (result == 0)
    {
      hold_registers[ADDR_Pass_Point] ++;
    }
  }

  hold_registers[ADDR_Complete_Point] ++;
  hold_registers[ADDR_Cur_Point_1] = cp;
  hold_registers[ADDR_Cur_Point_2] = cp;
}

// 三軸歸位
void Motor_Home(void)               // Core 0
{
  WORD  i;
  
  digitalWrite(DIR[z_axis], HIGH);
  digitalWrite(DIR[x_axis], LOW);
  digitalWrite(DIR[y_axis], HIGH);

  // Z 軸歸位
  for (i = 0; i < Z_LIMIT * mm_step_z; i ++)
  {
    if (digitalRead(HOME[z_axis]) == 1)
      break;
    
    digitalWrite(PUL[z_axis], HIGH);               
    delayMicroseconds(150);
    digitalWrite(PUL[z_axis], LOW);
    delayMicroseconds(150);
  }
  
  // Y 軸歸位
  for (i = 0; i < Y_LIMIT * mm_step_y; i ++)
  {
    if (digitalRead(HOME[y_axis]) == 1)
      break;
    
    digitalWrite(PUL[y_axis], HIGH);               
    delayMicroseconds(150);
    digitalWrite(PUL[y_axis], LOW);
    delayMicroseconds(150);
  }
  
  // X 軸歸位
  for (i = 0; i < X_LIMIT * mm_step_x; i ++)
  {
    if (digitalRead(HOME[x_axis]) == 1)
      break;

    digitalWrite(PUL[x_axis], HIGH);               
    delayMicroseconds(150);
    digitalWrite(PUL[x_axis], LOW);
    delayMicroseconds(150);
  }
}

// 移動三軸至邏輯零點
void Auto_Start_Set(void)             // Core 0
{
  int dd, target;
  
  target = (int) hold_registers[ADDR_X_Zero];
  if (hold_registers[ADDR_X_Cur] != target)
  {
    if (hold_registers[ADDR_X_Cur] > target)
    {
      dd = (int) hold_registers[ADDR_X_Cur] - target;
      Auto_Motor(x_axis, dd, dir_Xbackward); 
    }
    else if (hold_registers[ADDR_X_Cur] < target)
    {
      dd = target - (int) hold_registers[ADDR_X_Cur];
      Auto_Motor(x_axis, dd, dir_Xforward);
    }
    
    hold_registers[ADDR_X_Cur] = target;
  }
  
  target = (int) hold_registers[ADDR_Y_Zero];
  if (hold_registers[ADDR_Y_Cur] != target)
  {
    if (hold_registers[ADDR_Y_Cur] > target)
    {
      dd = (int) hold_registers[ADDR_Y_Cur] - target;
      Auto_Motor(y_axis, dd, dir_Ybackward);
    }
    else if (hold_registers[ADDR_Y_Cur] < target)
    {
      dd = target - (int) hold_registers[ADDR_Y_Cur];
      Auto_Motor(y_axis, dd, dir_Yforward);
    }
    
    hold_registers[ADDR_Y_Cur] = target;
  }
  
  target = (int) hold_registers[ADDR_Z_Zero] - (int) hold_registers[ADDR_Z_Offset];
  if (hold_registers[ADDR_Z_Cur] != target)
  {
    if (hold_registers[ADDR_Z_Cur] > target)
    {
      dd = (int) hold_registers[ADDR_Z_Cur] - target;
      Auto_Motor(z_axis, dd, dir_Zforward);
    }
    else if (hold_registers[ADDR_Z_Cur] < target)
    {
      dd = target - (int) hold_registers[ADDR_Z_Cur];
      Auto_Motor(z_axis, dd, dir_Zbackward);
    }
    
    hold_registers[ADDR_Z_Cur] = target;
  }
}

// 移動 X, Y 軸至測試點位
void Auto_Coordinate_set_xy(WORD x_target, WORD y_target)     // Core 0
{
  int dd, target;
  
  target = (int) x_target + (int) hold_registers[ADDR_X_Zero];
  
  if (hold_registers[ADDR_X_Cur] != target)
  {
    if (target > X_LIMIT)
    {
      dd = X_LIMIT - hold_registers[ADDR_X_Cur];
      Auto_Motor(x_axis, dd, dir_Xforward);
      target = X_LIMIT;
    }
    else if (target < 0)
    { 
      dd = (int) hold_registers[ADDR_X_Cur];
      Auto_Motor(x_axis, dd, dir_Xbackward);
      target = 0;
    }
    else if (hold_registers[ADDR_X_Cur] > target)
    {
      dd = (int) hold_registers[ADDR_X_Cur] - target;
      Auto_Motor(x_axis, dd, dir_Xbackward);
    }
    else if (hold_registers[ADDR_X_Cur] < target)
    {
      dd = target - (int) hold_registers[ADDR_X_Cur];
      Auto_Motor(x_axis, dd, dir_Xforward);
    }
    
    hold_registers[ADDR_X_Cur] = (WORD) target;
  }
  
  target = (int) y_target + (int) hold_registers[ADDR_Y_Zero];
  
  if (hold_registers[ADDR_Y_Cur] != target)
  {
    if (target > Y_LIMIT)
    {
      dd = Y_LIMIT - hold_registers[ADDR_Y_Cur];
      Auto_Motor(y_axis, dd, dir_Yforward);
      target = Y_LIMIT;
    }
    else if (target < 0)
    { 
      dd = (int) hold_registers[ADDR_Y_Cur];
      Auto_Motor(y_axis, dd, dir_Ybackward);
      target = 0;
    }
    else if (hold_registers[ADDR_Y_Cur] > target)
    {
      dd = (int) hold_registers[ADDR_Y_Cur] - target;
      Auto_Motor(y_axis, dd, dir_Ybackward);
      
    }
    else if (hold_registers[ADDR_Y_Cur] < target)
    {
      dd = target - (int) hold_registers[ADDR_Y_Cur];
      Auto_Motor(y_axis, dd, dir_Yforward);
    }
    
    hold_registers[ADDR_Y_Cur] = (WORD) target;
  }
}

// 移動 Z 軸下針與上針
void Auto_Coordinate_set_z(WORD z_target)         // Core 0
{
  int dd, target;
  
  if (z_target == hold_registers[ADDR_Z_Zero])
  {
    target = hold_registers[ADDR_Z_Zero];
    if (hold_registers[ADDR_Z_Cur] != target)
    {
      if (hold_registers[ADDR_Z_Cur] > target)
      {
        dd = (int) hold_registers[ADDR_Z_Cur] - target;
        Auto_Motor(z_axis, dd, dir_Zbackward);
      }
      else if (hold_registers[ADDR_Z_Cur] < target)
	      
      {
        dd = target - (int) hold_registers[ADDR_Z_Cur];
        Auto_Motor(z_axis, dd, dir_Zforward);
      }
      
      hold_registers[ADDR_Z_Cur] = (WORD) target;
    }
  }
  else if (z_target == hold_registers[ADDR_Z_Offset])
  {
    target = hold_registers[ADDR_Z_Zero] - hold_registers[ADDR_Z_Offset];
    
    if (hold_registers[ADDR_Z_Cur] != target)
    {
      if (hold_registers[ADDR_Z_Cur] > target)
      {
        dd = (int) hold_registers[ADDR_Z_Cur] - target;
        Auto_Motor(z_axis, dd, dir_Zbackward);
      }
      else if (hold_registers[ADDR_Z_Cur] < target)
      {
        dd = target - (int) hold_registers[ADDR_Z_Cur];
        Auto_Motor(z_axis, dd, dir_Zforward);
      }
      
      hold_registers[ADDR_Z_Cur] = (WORD) target;
    }
  }
}

// 移動 X, Y, Z 軸
void Auto_Motor(BYTE c_axis, int mm_move, BYTE dir_move)      // Core 0
{
  int   i, j;
  
  switch (c_axis)
  {
    case x_axis: j = mm_move * mm_step_x; break;
    case y_axis: j = mm_move * mm_step_y; break;
    case z_axis: j = mm_move * mm_step_z; break;
  }
  
  digitalWrite(DIR[c_axis], dir_move);
  
  for (i = 0; i < j; i ++)
  {
    if (Stop_Flag == true)
      break;

    digitalWrite(PUL[c_axis], HIGH);               
    delayMicroseconds(150);
    digitalWrite(PUL[c_axis], LOW);  //移動速度 原200
    delayMicroseconds(150);
  }
}

// 執行測試功能
WORD Prob_function(WORD ff)               // Core 0
{
  int result;
  
  // 測試延遲時間
  delay(hold_registers[ADDR_Probe_Delay]);
  
  switch (ff)
  {
    case Short_ckt_V:
      pinMode(Probe_DI, INPUT_PULLDOWN);
      result = digitalRead(Probe_DI);
      
      if (result == HIGH)
        return 0; 
      else  return 1;
      
      break;
      
    case Short_ckt_G:
      pinMode(Probe_DI, INPUT_PULLUP);
      result = digitalRead(Probe_DI);
      
      if (result == HIGH)
        return 1;
      else  return 0;
      
      break;
      
    case Analog_test:
      return ((WORD) analogRead(Probe_AI));
      break;
      
    default: return 1;
  }
}