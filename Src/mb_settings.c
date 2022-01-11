

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


enum SAVE_MAGIC_CMD {
    SAVE_CMD_RESET  = 0x8012,
    SAVE_CMD_MAGIC1 = 0xA321,
    SAVE_CMD_MAGIC2 = 0xB456,
};

enum SAVE_STATUS_MACHINE {
    SAVE_IDLE = 1,
    SAVE_MAGIC1 = 2,
    SAVE_MAGIC2 = 3,
    SAVE_DONE = 4,
};

static enum SAVE_STATUS_MACHINE saveMachine;
static int16_t saveCount = 0;

static modbusHandler_t MBSettingsH;
static uint16_t        ModusSlaveDataBuffer[(sizeof(struct SystemSettings)/sizeof(uint16_t))];
static uint16_t        SysSettings[(sizeof(struct SystemSettings)/sizeof(uint16_t))];
struct SystemSettings*  pSysSettings=(struct SystemSettings*)SysSettings;



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

static void checkAndUpdateSysTime(void) {
  uint32_t valNew = MBSettingsH.u16regs[R57_SYS_TIME_WH] * 65536 +  MBSettingsH.u16regs[R56_SYS_TIME_WL];

  if (valNew != 0) {
    pSysSettings->myTimeLow = MBSettingsH.u16regs[R56_SYS_TIME_WL];
    pSysSettings->myTimeHigh = MBSettingsH.u16regs[R57_SYS_TIME_WH];
    syncRTCTime(valNew);
  }
}

static void checkAndUpdateManualOp(void) {
  if (MBSettingsH.u16regs[R58_MANUAL_CMD] == 0)
    return;

  const uint8_t port = MBSettingsH.u16regs[R58_MANUAL_CMD] >> 8;
  const uint8_t value = MBSettingsH.u16regs[R58_MANUAL_CMD] & 0xFF;
  DEBUG_PRINTF("set Y%d: %d\n", port-1, value);
  switch (port) {
  case 1:
      break;
  case 2:
      break;
  case 3:
      break;
  case 4:
      break;
  case 5:
      break;
  case 6:
      break;
  case 7:
      break;
  case 8:
      break;
  case 9:
      break;
  case 10:
      break;
  case 11:
      break;
  case 12:
      break;
  }

  switch(saveMachine) {
  case SAVE_IDLE:
      if (MBSettingsH.u16regs[R58_MANUAL_CMD] == SAVE_CMD_RESET) {
        saveMachine = SAVE_MAGIC1;
        saveCount = 0;
      }
      break;
  case SAVE_MAGIC1:
      if (MBSettingsH.u16regs[R58_MANUAL_CMD] == SAVE_CMD_MAGIC1)
        saveMachine = SAVE_MAGIC2;
      break;
  case SAVE_MAGIC2:
      if (MBSettingsH.u16regs[R58_MANUAL_CMD] == SAVE_CMD_MAGIC2) {
        saveMachine = SAVE_DONE;
        updateAndSaveCfg();
      }
  default:
      if (MBSettingsH.u16regs[R58_MANUAL_CMD] == SAVE_CMD_RESET) {
        saveMachine = SAVE_IDLE;
      }
      break;
  }
}

static void updateSettings(void) {
  if (MBSettingsH.u16regs[R00_TOWER_ID] != pSysSettings->towerID) {
    DEBUG_PRINTF("Update TowerID: %d\n", MBSettingsH.u16regs[R00_TOWER_ID]);
    pSysSettings->towerID = MBSettingsH.u16regs[R00_TOWER_ID];
  }

  if (MBSettingsH.u16regs[R29_VALVE_SWITCH_TIME] != pSysSettings->valveSwitchTime)
  {
    DEBUG_PRINTF("Update ValeSwitch Time: %d\n", MBSettingsH.u16regs[R29_VALVE_SWITCH_TIME]);
    pSysSettings->valveSwitchTime = MBSettingsH.u16regs[R29_VALVE_SWITCH_TIME];
  }

  for (int i = R36_RUN_MODE; i <= R55_FIELD_MASK; i++) {
    if (SysSettings[i] != MBSettingsH.u16regs[i]) {
      DEBUG_PRINTF("Update R%02d: value:%d(0x%04X)\n", i, MBSettingsH.u16regs[i], MBSettingsH.u16regs[i]);
      SysSettings[i] = MBSettingsH.u16regs[i];
    }
  }
}

static void MbSettingsProc(void) {
  checkAndUpdateSysTime();
  checkAndUpdateManualOp();
  updateSettings();
  memcpy(ModusSlaveDataBuffer, pSysSettings, sizeof(SysSettings));
}

void InitMbSettings(void) {
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
