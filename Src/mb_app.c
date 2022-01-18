
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
#include "sysSettings.h"
#include "sysIO.h"
#include "debug.h"
#include "defaultConfig.h"

modbusHandler_t ModbusMasterH;
uint16_t        u16EC;
uint16_t        u16PH;
uint16_t        u16Temp1Array[4];
uint16_t        u16Temp2Array[4];
uint16_t        u16DIO;

void InitMbReadSensors(void);
static void MbReadSensorsProc(void);
void StartMbReadSensorsTask(void *argument);


void StartMbReadSensorsTask(void *argument)
{
  for(;;) {
    MbReadSensorsProc();
    osDelay(10);
  }
}

static bool isCoTemp(void)
{
  return ((SysStatus[R55_FIELD_MASK] & (1 << 0)) != 0);
}

static void readEC(void)
{
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
    INFO_PRINTF(RTT_CTRL_TEXT_BRIGHT_RED "read EC failed\r\n");
    INFO_PRINTF(RTT_CTRL_RESET);
  } else {
    DEBUG_PRINTF("read EC:0x%04X(%d)\r\n", u16EC, u16EC);
    SysStatus[R03_EC] = u16EC;
  }
}

static void readPH(void)
{
  modbus_t telegram;
  uint32_t u32NotificationValue;

  telegram.u8id = SENSOR_PH; // slave address
  telegram.u8fct = MB_FC_READ_REGISTERS; // function code 03, only support the code
  telegram.u16RegAdd = 6; // start address in slave, 40007
  telegram.u16CoilsNo = 1; // number of elements (coils or registers) to read
  telegram.u16reg = &u16PH; // pointer to a memory array in the Arduino
  ModbusQuery(&ModbusMasterH, telegram);
  u32NotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  if (u32NotificationValue != (uint32_t)ERR_OK_QUERY) {
    // error handler
    INFO_PRINTF(RTT_CTRL_TEXT_BRIGHT_RED "read PH failed\r\n");
    INFO_PRINTF(RTT_CTRL_RESET);
  } else {
    DEBUG_PRINTF("read PH:0x%04X(%d)\r\n", u16PH, u16PH);
    SysStatus[R04_PH] = u16PH;
  }
}

static void readTemp(void)
{
  modbus_t telegram;
  uint32_t u32NotificationValue;
  bool coTempSensors = isCoTemp();

  telegram.u8id = SENSOR_TEMP1;
  telegram.u8fct = MB_FC_READ_REGISTERS; // function code
  telegram.u16RegAdd = 40; // 0x28
  telegram.u16CoilsNo = (coTempSensors) ? 2 : 4;
  telegram.u16reg = u16Temp1Array;
  ModbusQuery(&ModbusMasterH, telegram);
  u32NotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  if (u32NotificationValue != (uint32_t)ERR_OK_QUERY) {
    // error handler
    INFO_PRINTF(RTT_CTRL_TEXT_BRIGHT_RED "read temperature failed\n");
    INFO_PRINTF(RTT_CTRL_RESET);
  }
  else {
    DEBUG_PRINTF("read Temp1 ch0:%d ch1:%d ch2:%d ch3:%d\n",
    u16Temp1Array[0], u16Temp1Array[1], u16Temp1Array[2], u16Temp1Array[3]);
    SysStatus[R01_TEMP1_IN] = u16Temp1Array[0];
    SysStatus[R02_TEMP1_OUT] = u16Temp1Array[1];
    if (coTempSensors) {
      SysStatus[R07_TEMP2_IN] = u16Temp1Array[0];
      SysStatus[R08_TEMP2_OUT] = u16Temp1Array[1];
    } else {
      SysStatus[R07_TEMP2_IN] = u16Temp1Array[2];
      SysStatus[R08_TEMP2_OUT] = u16Temp1Array[3];
    }
  }

  if (coTempSensors) {
      SysStatus[R09_TEMP3_IN] = u16Temp1Array[0];
      SysStatus[R10_TEMP3_OUT] = u16Temp1Array[1];
      SysStatus[R11_TEMP4_IN] = u16Temp1Array[0];
      SysStatus[R12_TEMP4_OUT] = u16Temp1Array[1];
  }
  else {
    telegram.u8id = SENSOR_TEMP2;
    telegram.u8fct = MB_FC_READ_REGISTERS; // function code
    telegram.u16RegAdd = 40; // 0x28
    telegram.u16CoilsNo = 4;
    telegram.u16reg = u16Temp2Array;
    ModbusQuery(&ModbusMasterH, telegram);
    u32NotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if (u32NotificationValue != (uint32_t)ERR_OK_QUERY) {
    // error handler
      INFO_PRINTF(RTT_CTRL_TEXT_BRIGHT_RED "read temperature failed\r\n");
      INFO_PRINTF(RTT_CTRL_RESET);
    } else {
      DEBUG_PRINTF("read Temp2 ch0:%d ch1:%d ch2:%d ch3:%d\r\n",
      u16Temp2Array[0], u16Temp2Array[1], u16Temp2Array[2], u16Temp2Array[3]);
      SysStatus[R09_TEMP3_IN] = u16Temp1Array[0];
      SysStatus[R10_TEMP3_OUT] = u16Temp1Array[1];
      SysStatus[R11_TEMP4_IN] = u16Temp1Array[2];
      SysStatus[R12_TEMP4_OUT] = u16Temp1Array[3];
    }
  }
}

static void SetDO(void)
{
  modbus_t telegram;
  uint32_t u32NotificationValue;

  u16DIO = sysOut._out;
  telegram.u8id = MB_DIO;
  telegram.u8fct = MB_FC_WRITE_REGISTER; // function code
  telegram.u16RegAdd = 0;
  telegram.u16CoilsNo = 1;
  telegram.u16reg = &u16DIO;
  ModbusQuery(&ModbusMasterH, telegram);
  u32NotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  if (u32NotificationValue != (uint32_t)ERR_OK_QUERY) {
    // error handler
    INFO_PRINTF(RTT_CTRL_TEXT_BRIGHT_RED "write DIO failed\n");
    INFO_PRINTF(RTT_CTRL_RESET);
  } else {
    DEBUG_PRINTF("Write DIO:0x%04X\n", u16DIO);
  }
}

static void GetDI(void)
{
  modbus_t telegram;
  uint32_t u32NotificationValue;

  u16DIO = 0;
  telegram.u8id = MB_DIO;
  telegram.u8fct = MB_FC_READ_REGISTERS; // function code
  telegram.u16RegAdd = 0x04;
  telegram.u16CoilsNo = 1;
  telegram.u16reg = &u16DIO;
  ModbusQuery(&ModbusMasterH, telegram);
  u32NotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  if (u32NotificationValue != (uint32_t)ERR_OK_QUERY) {
    // error handler
    INFO_PRINTF(RTT_CTRL_TEXT_BRIGHT_RED "read DIO failed\n");
    INFO_PRINTF(RTT_CTRL_RESET);
  } else {
    DEBUG_PRINTF("read DIO:0x%04X\n", u16DIO);
    sysIn._in = (u16DIO & 0xFF);
    SysStatus[R24_DIGITAL_INPUT] = u16DIO;
  }
}

static void MbReadSensorsProc(void) {
  readEC();
  osDelay(2);
  readPH();
  osDelay(2);
  readTemp();
  osDelay(2);
  GetDI();
  osDelay(2);
  SetDO();
}

void InitMbReadSensors(void) {
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
  ModbusMasterH.u16timeOut = 400;
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
