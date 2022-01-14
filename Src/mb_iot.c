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
#include "sysIO.h"
#include "swTimer.h"
#include "debug.h"
#include "defaultConfig.h"

static modbusHandler_t MBIoTH;
static uint16_t        mbIoTDataBuffer[48];

enum UPLOAD_STATE_MACHINE {
  UPLOAD_OPERATION_INIT,
  FIRST_UPDATE,
  UPLOAD_IDLE_DATA,
  UPLOAD_DATA,
  TEST_UPLOAD_DATA,
  TURN_OFF_POWER,
};

struct UploadOperationData {
  uint32_t  waitSecondCounter;        /* for waiting counter */
  uint32_t  waitMinuteCounter;
};

static int IsPowerOff(void);
void InitMbIoT(void);
void modbusIoTProc(void);

static enum UPLOAD_STATE_MACHINE upState;
static struct UploadOperationData upData;

static int IsPowerOff(void)
{
  const uint8_t PowerOffHour = SysSettings[R54_NBIOT_POWEROFF_TIME] / 100;
  const uint8_t powerOffMinute = SysSettings[R54_NBIOT_POWEROFF_TIME] % 100;
  return 0;
}

void modbusIoTProc(void) {
  if (upData.waitMinuteCounter != getTimeMinuteCount()) {
    upData.waitMinuteCounter = getTimeMinuteCount();
    if (IsPowerOff()) {
      upState = TURN_OFF_POWER;
    }
  }

  if (SysSettings[R36_RUN_MODE] != MODE_RUNNING) {
      upState = UPLOAD_OPERATION_INIT;
      return;
  }

  switch(upState) {
  case UPLOAD_OPERATION_INIT:
    upData.waitSecondCounter = getTimeCount();
    upState = FIRST_UPDATE;

    NBIOT_ON_CTRL = TURN_ON_NBIOT;

  case FIRST_UPDATE:
    if ((getTimeCount() - upData.waitSecondCounter) >= WAIT_FIRST_UPLOAD) {
      upState = UPLOAD_DATA;
    }
    break;

  case UPLOAD_IDLE_DATA:
    if ((getTimeCount() - upData.waitSecondCounter) >= UPLOAD_PERIOD) {
      upState = UPLOAD_DATA;
    }
    break;

  case UPLOAD_DATA:
    upState = UPLOAD_IDLE_DATA;
    upData.waitSecondCounter = getTimeCount();
    break;

  case TEST_UPLOAD_DATA:
    upState = UPLOAD_IDLE_DATA;
    upData.waitSecondCounter = getTimeCount();
    break;

  case TURN_OFF_POWER:
    if ((getTimeCount() - upData.waitSecondCounter) >= UPLOAD_PERIOD) {
      upState = FIRST_UPDATE;
      NBIOT_ON_CTRL = TURN_ON_NBIOT;
    }
    break;
  } /* end of switch(upState) */
}

void InitMbIoT(void)
{
  MBIoTH.uModbusType = MB_MASTER;
  MBIoTH.xTypeHW = USART_HW;
  MBIoTH.port = &MODBUS_IOT_UART;
  MBIoTH.u8id = 0; // for Mastter
  MBIoTH.u16timeOut = 1000;
  MBIoTH.EN_Port = NULL; // No RS485
  MBIoTH.u8coils = NULL;
  MBIoTH.u8coilsmask = NULL;
  MBIoTH.u16coilsize = 0;
  MBIoTH.u16regs = mbIoTDataBuffer;
  MBIoTH.u16regsize = sizeof(mbIoTDataBuffer)/sizeof(uint16_t);
  MBIoTH.u8regsmask = NULL;
  //Initialize Modbus library
  ModbusInit(&MBIoTH);
  //Start capturing traffic on serial Port
  ModbusStart(&MBIoTH);
  upState = UPLOAD_OPERATION_INIT;
  upData.waitSecondCounter = getTimeCount();
  upData.waitMinuteCounter = getTimeMinuteCount();
}
