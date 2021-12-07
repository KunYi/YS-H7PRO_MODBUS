
/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "semphr.h"
#include "Modbus.h"
#include "sysStatus.h"
#include "SEGGER_RTT.h"

#define MODBUS_SLAVE_ID	       (1)
#define TOWER_ID               (12345)

#define MODBUS_MASTER_UART     huart4 /* RS485 Port 1 */
#define MODBUS_SLAVE_UART      huart8 /* RS485 port 2, PE0, PE1 UART8 */


/* sensor's slave ID */
#define SENSOR_EC              (9)
#define SENSOR_PH              (8)
#define SENSOR_TEMP1           (3)
#define SENSOR_TEMP2           (4)

#define MB_DIO                 (20)

modbusHandler_t ModbusSlaveH;
modbusHandler_t ModbusMasterH;
uint8_t         ModbusSlaveCoilBuffer[10];
uint16_t        ModusSlaveDataBuffer[48];
uint16_t        SysStatus[48];
uint16_t        u16EC;
uint16_t        u16PH;
uint16_t        u16Temp1Array[4];
uint16_t        u16Temp2Array[4];
uint16_t        u16DIO;
struct SystemStatus*  pSysStatus=(struct SystemStatus*)SysStatus;

void InitModbusMaster(void);
void InitModbusSlave(void);
static void ModbusSlaveProc(void);
static void ModbusMasterProc(void);

void StartModbusMasterTask(void *argument);
void StartModbusSlaveTask(void *argument);

void StartModbusMasterTask(void *argument)
{
  for(;;) {
    ModbusMasterProc();
    osDelay(10);
  }
}

void StartModbusSlaveTask(void *argument)
{
  for(;;) {
    xSemaphoreTake(ModbusSlaveH.ModBusSphrHandle , portMAX_DELAY);
    ModbusSlaveProc();
    xSemaphoreGive(ModbusSlaveH.ModBusSphrHandle);
    osDelay(100);
  }
}

static void ModbusMasterProc(void) {
  modbus_t telegram;
  uint32_t u32NotificationValue;

  telegram.u8id = SENSOR_EC; // slave address
  telegram.u8fct = MB_FC_READ_REGISTERS; // function code 03, only support the code
  telegram.u16RegAdd = 2; // start address in slave, 40003
  telegram.u16CoilsNo = 1; // number of elements (coils or registers) to read
  telegram.u16reg = &u16EC; // pointer to a memory array in the Arduino
  ModbusQuery(&ModbusMasterH, telegram);
  u32NotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  if (u32NotificationValue != (uint32_t)ERR_OK_QUERY) {
    // error handler
    SEGGER_RTT_printf(0, RTT_CTRL_TEXT_BRIGHT_RED "read EC failed\r\n");
    SEGGER_RTT_printf(0, RTT_CTRL_RESET);		
  } else {
    SEGGER_RTT_printf(0, "read EC:0x%04X(%d)\r\n", u16EC, u16EC);
    pSysStatus->EC = u16EC;
  }
  osDelay(1);

  telegram.u8id = SENSOR_PH; // slave address
  telegram.u8fct = MB_FC_READ_REGISTERS; // function code 03, only support the code
  telegram.u16RegAdd = 6; // start address in slave, 40007
  telegram.u16CoilsNo = 1; // number of elements (coils or registers) to read
  telegram.u16reg = &u16PH; // pointer to a memory array in the Arduino
  ModbusQuery(&ModbusMasterH, telegram);
  u32NotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  if (u32NotificationValue != (uint32_t)ERR_OK_QUERY) {
    // error handler
    SEGGER_RTT_printf(0, RTT_CTRL_TEXT_BRIGHT_RED "read PH failed\r\n");
    SEGGER_RTT_printf(0, RTT_CTRL_RESET);
  } else {
    SEGGER_RTT_printf(0, "read PH:0x%04X(%d)\r\n", u16PH, u16PH);
    pSysStatus->PH = u16PH;
  }
  osDelay(1);

  telegram.u8id = SENSOR_TEMP1;
  telegram.u8fct = MB_FC_READ_REGISTERS; // function code
  telegram.u16RegAdd = 40; // 0x28
  telegram.u16CoilsNo = 4;
  telegram.u16reg = u16Temp1Array;
  ModbusQuery(&ModbusMasterH, telegram);
  u32NotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  if (u32NotificationValue != (uint32_t)ERR_OK_QUERY) {
    // error handler
    SEGGER_RTT_printf(0, RTT_CTRL_TEXT_BRIGHT_RED "read temperature failed\r\n");
    SEGGER_RTT_printf(0, RTT_CTRL_RESET);	
  } else {
    SEGGER_RTT_printf(0, "read Temp1 ch0:%d ch1:%d ch2:%d ch3:%d\r\n",
    u16Temp1Array[0], u16Temp1Array[1], u16Temp1Array[2], u16Temp1Array[3]);
    pSysStatus->TempIn1 = u16Temp1Array[0];
    pSysStatus->TempOutput1 = u16Temp1Array[1];
    pSysStatus->TempIn2 = u16Temp1Array[2];
    pSysStatus->TempOutput2 = u16Temp1Array[2];
  }
  osDelay(1);

  telegram.u8id = SENSOR_TEMP2;
  telegram.u8fct = MB_FC_READ_REGISTERS; // function code
  telegram.u16RegAdd = 40; // 0x28
  telegram.u16CoilsNo = 4;
  telegram.u16reg = u16Temp2Array;
  ModbusQuery(&ModbusMasterH, telegram);
  u32NotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  if (u32NotificationValue != (uint32_t)ERR_OK_QUERY) {
    // error handler
    SEGGER_RTT_printf(0, RTT_CTRL_TEXT_BRIGHT_RED "read temperature failed\r\n");
    SEGGER_RTT_printf(0, RTT_CTRL_RESET);
  } else {
    SEGGER_RTT_printf(0, "read Temp2 ch0:%d ch1:%d ch2:%d ch3:%d\r\n",
    u16Temp2Array[0], u16Temp2Array[1], u16Temp2Array[2], u16Temp2Array[3]);
    pSysStatus->TempIn3 = u16Temp2Array[0];
    pSysStatus->TempOutput3 = u16Temp2Array[1];
    pSysStatus->TempIn4 = u16Temp2Array[2];
    pSysStatus->TempOutput4 = u16Temp2Array[2];
  }
  osDelay(1);

  telegram.u8id = MB_DIO;
  telegram.u8fct = MB_FC_WRITE_REGISTER; // function code
  telegram.u16RegAdd = 0;
  telegram.u16CoilsNo = 1;
  telegram.u16reg = &u16DIO;
  ModbusQuery(&ModbusMasterH, telegram);
  u16DIO++;
  u32NotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  if (u32NotificationValue != (uint32_t)ERR_OK_QUERY) {
    // error handler
    SEGGER_RTT_printf(0, RTT_CTRL_TEXT_BRIGHT_RED "write DIO failed\r\n");
    SEGGER_RTT_printf(0, RTT_CTRL_RESET);	
  } else {
    SEGGER_RTT_printf(0, "write DIO:%04X(%03d)\r\n", u16DIO, u16DIO);
  }
}

static void ModbusSlaveProc(void) {
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, (GPIO_PinState)(ModbusSlaveH.u8coils[0] & (1<<0)));
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, (GPIO_PinState)(ModbusSlaveH.u8coils[0] & (1<<1)));
    //printf("reg 0:0x%04X, reg 1:0x%04X\r\n", ModbusSlaveH.u16regs[0], ModbusSlaveH.u16regs[1]);
    memcpy(ModusSlaveDataBuffer, pSysStatus, sizeof(SysStatus));
}


void InitModbusMaster(void) {
  u16EC = 0;
  u16PH = 0;
  for (int i = 0; i < 4; i++) {
    u16Temp1Array[i] = 0;
    u16Temp2Array[i] = 0;
  }

  ModbusMasterH.uModbusType = MB_MASTER;
  ModbusMasterH.xTypeHW = USART_HW;
  ModbusMasterH.port = &MODBUS_MASTER_UART;
  ModbusMasterH.u8id = 0; // for Mastter
  ModbusMasterH.u16timeOut = 1000;
  ModbusMasterH.EN_Port = NULL; // No RS485
  ModbusMasterH.u8coils = NULL;
  ModbusMasterH.u8coilsmask = NULL;
  ModbusMasterH.u16coilsize = 0;
  ModbusMasterH.u16regs = &u16EC;
  ModbusMasterH.u16regsize = 1;
  ModbusMasterH.u8regsmask = NULL;
  //Initialize Modbus library
  ModbusInit(&ModbusMasterH);
  //Start capturing traffic on serial Port
  ModbusStart(&ModbusMasterH);
}

void InitModbusSlave(void) {
  memset(SysStatus, 0, sizeof(SysStatus));
  pSysStatus->TowerNo = TOWER_ID;
  pSysStatus->TempIn1 = 189;           /* 18.9C */
  pSysStatus->TempOutput1 = 201;       /* 20.1C */
  pSysStatus->EC = 1234;               /* EC: 1234 us/cm */
  pSysStatus->PH = 720;                /* PH 7.2 */
  memcpy(ModusSlaveDataBuffer, pSysStatus, sizeof(SysStatus));
  /* Modbus Slave initialization */
  ModbusSlaveH.uModbusType = MB_SLAVE;
  ModbusSlaveH.xTypeHW = USART_HW;
  ModbusSlaveH.port =  &MODBUS_SLAVE_UART; // This is the UART port connected to STLINK in the NUCLEO F429
  ModbusSlaveH.u8id = MODBUS_SLAVE_ID; //slave ID, always different than zero
  ModbusSlaveH.u16timeOut = 1000;
  ModbusSlaveH.EN_Port = NULL; // No RS485
  //ModbusH2.EN_Port = LD2_GPIO_Port; // RS485 Enable
  //ModbusH2.EN_Pin = LD2_Pin; // RS485 Enable
  // for coils
  ModbusSlaveH.u8coils = ModbusSlaveCoilBuffer;
  ModbusSlaveH.u8coilsmask = NULL;
  ModbusSlaveH.u16coilsize = sizeof(ModbusSlaveCoilBuffer);
  // for regs
  ModbusSlaveH.u16regs = ModusSlaveDataBuffer;
  ModbusSlaveH.u16regsize= sizeof(ModusSlaveDataBuffer)/sizeof(ModusSlaveDataBuffer[0]);
  ModbusSlaveH.u8regsmask = NULL;
  //Initialize Modbus library
  ModbusInit(&ModbusSlaveH);
  //Start capturing traffic on serial Port
  ModbusStart(&ModbusSlaveH);
}
