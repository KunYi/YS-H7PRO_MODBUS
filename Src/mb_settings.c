

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "semphr.h"
#include "Modbus.h"
#include "sysSettings.h"
#include "SEGGER_RTT.h"

static modbusHandler_t MBSettingsH;
static uint16_t        SysSettings[(sizeof(struct SystemSettings)/sizeof(uint16_t))];
static uint16_t        ModusSlaveDataBuffer[(sizeof(struct SystemSettings)/sizeof(uint16_t))];
static struct SystemSettings*  pSysSettings=(struct SystemSettings*)SysSettings;

void InitMbSettings(void);
void StartMbSettingsTask(void *argument);

static void MbSettingsProc(void);

void StartMbSettingsTask(void *argument)
{
  for(;;) {
    xSemaphoreTake(MBSettingsH.ModBusSphrHandle , portMAX_DELAY);
    MbSettingsProc();
    xSemaphoreGive(MBSettingsH.ModBusSphrHandle);
    osDelay(10);
  }
}

static void MbSettingsProc(void) {
    // HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, (GPIO_PinState)(MBSettingsH.u8coils[0] & (1<<0)));
    // HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, (GPIO_PinState)(MBSettingsH.u8coils[0] & (1<<1)));
    //printf("reg 0:0x%04X, reg 1:0x%04X\r\n", ModbusSlaveH.u16regs[0], ModbusSlaveH.u16regs[1]);
    memcpy(ModusSlaveDataBuffer, pSysSettings, sizeof(SysSettings));
}

void InitMbSettings(void) {
  memset(&SysSettings, 0, sizeof(SysSettings));
  memcpy(ModusSlaveDataBuffer, pSysSettings, sizeof(SysSettings));
  /* Modbus Slave initialization */
  MBSettingsH.uModbusType = MB_SLAVE;
  MBSettingsH.xTypeHW = USART_HW;
  MBSettingsH.port =  &MODBUS_SETTINGS_UART; // This is the UART port connected to STLINK in the NUCLEO F429
  MBSettingsH.u8id = MODBUS_SLAVE_ID; //slave ID, always different than zero
  MBSettingsH.u16timeOut = 1000;
  MBSettingsH.EN_Port = NULL; // No RS485
  //ModbusH2.EN_Port = LD2_GPIO_Port; // RS485 Enable
  //ModbusH2.EN_Pin = LD2_Pin; // RS485 Enable
  // for coils
  MBSettingsH.u8coils = NULL;
  MBSettingsH.u8coilsmask = NULL;
  MBSettingsH.u16coilsize = 0;
  // for regs
  MBSettingsH.u16regs = ModusSlaveDataBuffer;
  MBSettingsH.u16regsize= sizeof(ModusSlaveDataBuffer)/sizeof(ModusSlaveDataBuffer[0]);
  MBSettingsH.u8regsmask = NULL;
  //Initialize Modbus library
  ModbusInit(&MBSettingsH);
  //Start capturing traffic on serial Port
  ModbusStart(&MBSettingsH);
}
