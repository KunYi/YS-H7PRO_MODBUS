
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
#include "defaultConfig.h"
#include "rtc.h"
#include "mytime.h"
#include "cfg.h"
#include "debug.h"
#include "sysIO.h"
#include "cleanProc.h"


enum SAVE_MAGIC_CMD {
    SAVE_CMD_MAGIC = 0xA321,
};

static modbusHandler_t  MBSettingsH;
static uint16_t         ModusSlaveDataBuffer[MAX_SETTINGS_REG];
volatile uint16_t       SysStatus[MAX_SETTINGS_REG];

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

static void syncRTCTime(const int32_t val) {
  struct MYTIME mytime={0};
  epoch_to_datetime(val, &mytime);

  DEBUG_PRINTF("to set RTC %0.2d/%0.2d/%0.4d, %0.2d:%0.2d:%0.2d\n",
                mytime.month, mytime.day, mytime.year,
                mytime.hour, mytime.minute, mytime.second );

  RTC_TimeTypeDef sTime = { .Hours = mytime.hour,
                            .Minutes = mytime.minute,
                            .Seconds =  mytime.second,
                            .DayLightSaving = RTC_DAYLIGHTSAVING_NONE,
                            .StoreOperation = RTC_STOREOPERATION_RESET
                          };
  HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

  RTC_DateTypeDef sDate = { .Month = mytime.month,
                            .Date = mytime.day,
                            .Year = (mytime.year - 2000),
                            .WeekDay = RTC_WEEKDAY_MONDAY
                          };
  HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

extern osMutexId_t mutexSysMyTime_id;
static void checkAndUpdateSysTime(void) {
  uint32_t valNew = MBSettingsH.u16regs[R57_SYS_TIME_WH] * 65536 +  MBSettingsH.u16regs[R56_SYS_TIME_WL];

  if (valNew != 0) {
    osMutexAcquire(mutexSysMyTime_id, osWaitForever);
    SysStatus[R34_SYS_TIME_RL] = MBSettingsH.u16regs[R56_SYS_TIME_WL];
    SysStatus[R35_SYS_TIME_RH] = MBSettingsH.u16regs[R57_SYS_TIME_WH];
    syncRTCTime(valNew);
    osMutexRelease(mutexSysMyTime_id);
  }
}

static void opSaveMachine(const uint16_t cmd)
{
  if (cmd == SAVE_CMD_MAGIC) {
    DEBUG_PRINTF("Save CFG\n");
    updateAndSaveCfg();
  }
}

static void setManualOutput(const uint16_t cmd)
{
  const uint8_t port = cmd >> 8;
  const uint8_t value = cmd & 0xFF;
  if (port > 13)
    return;

  DEBUG_PRINTF("set Y%d: %d\n", port-1, value);
  switch (port) {
  case 1:
      DO0 = (value) ? 1 : 0;
      break;
  case 2:
      DO1 = (value) ? 1 : 0;
      break;
  case 3:
      DO2 = (value) ? 1 : 0;
      break;
  case 4:
      DO3 = (value) ? 1 : 0;
      break;
  case 5:
      DO4 = (value) ? 1 : 0;
      break;
  case 6:
      DO5 = (value) ? 1 : 0;
      break;
  case 7:
      DO6 = (value) ? 1 : 0;
      break;
  case 8:
      DO7 = (value) ? 1 : 0;
      break;
  case 9:
      DO8 = (value) ? 1 : 0;
      break;
  case 10:
      DO9 = (value) ? 1 : 0;
      break;
  case 11:
      DO10 = (value) ? 1 : 0;
      break;
  case 12:
      DO11 = (value) ? 1 : 0;
      break;
  }
}

static void checkAndUpdateManualOp(void) {
  if (MBSettingsH.u16regs[R58_MANUAL_CMD] == 0)
    return;
  setManualOutput(MBSettingsH.u16regs[R58_MANUAL_CMD]);
  opSaveMachine(MBSettingsH.u16regs[R58_MANUAL_CMD]);
}

static void updateSettingsValue(void) {
  if (MBSettingsH.u16regs[R00_TOWER_ID] != SysStatus[R00_TOWER_ID]) {
    DEBUG_PRINTF("Update TowerID: %d\n", MBSettingsH.u16regs[R00_TOWER_ID]);
    SysStatus[R00_TOWER_ID] = MBSettingsH.u16regs[R00_TOWER_ID];
  }

  if (MBSettingsH.u16regs[R29_VALVE_SWITCH_TIME] != SysStatus[R29_VALVE_SWITCH_TIME])
  {
    if (MBSettingsH.u16regs[R29_VALVE_SWITCH_TIME] == 0) {
      resetCumulateDrainTime();
    }
    DEBUG_PRINTF("Reset ValeSwitch Time: %d\n", MBSettingsH.u16regs[R29_VALVE_SWITCH_TIME]);
    SysStatus[R29_VALVE_SWITCH_TIME] = MBSettingsH.u16regs[R29_VALVE_SWITCH_TIME];
  }

  for (int i = R36_RUN_MODE; i <= R55_FIELD_MASK; i++) {
    if (SysStatus[i] != MBSettingsH.u16regs[i]) {
      DEBUG_PRINTF("Update R%02d: value:%d(0x%04X)\n", i, MBSettingsH.u16regs[i], MBSettingsH.u16regs[i]);
      SysStatus[i] = MBSettingsH.u16regs[i];
    }
  }
}

static void updateModbusMappingRegisters(void) {
  for (int i = 0; i < MAX_SETTINGS_REG; i++)
	  ModusSlaveDataBuffer[i] = SysStatus[i];
}

static void MbSettingsProc(void) {
  checkAndUpdateSysTime();
  checkAndUpdateManualOp();
  updateSettingsValue();
  updateModbusMappingRegisters();
}

void InitMbSettings(void) {
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
  updateModbusMappingRegisters();
  //Initialize Modbus library
  ModbusInit(&MBSettingsH);
  //Start capturing traffic on serial Port
  ModbusStart(&MBSettingsH);
}
