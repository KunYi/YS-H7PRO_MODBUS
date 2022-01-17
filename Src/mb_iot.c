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

#define NBIOT_MAX_REGISTERS   (48)
#define SUPPORT_UPDATE_REGISTERS (23)
static modbusHandler_t MBIoTH;
static uint16_t        mbIoTDataBuffer[NBIOT_MAX_REGISTERS];

enum UPLOAD_STATE_MACHINE {
  UPLOAD_OPERATION_INIT,
  FIRST_UPDATE,
  UPLOAD_IDLE_DATA,
  UPLOAD_DATA,
  TEST_UPLOAD_DATA,
  WAIT_UPDATA,
  TURN_OFF_POWER,
};

struct UploadOperationData {
  uint32_t  waitSecondCounter;        /* for waiting counter */
  uint32_t  waitMinuteCounter;
};

void InitMbIoT(void);
void modbusIoTProc(void);

static enum UPLOAD_STATE_MACHINE upState;
static struct UploadOperationData upData;

static void updateDataToNBIOT(void)
{
  modbus_t telegram;
  telegram.u8id = MB_NBIOT; // slave address
  telegram.u8fct = MB_FC_WRITE_MULTIPLE_REGISTERS; // function code 03, only support the code
  telegram.u16RegAdd = 0; // start address in slave, 40001
  telegram.u16CoilsNo = NBIOT_MAX_REGISTERS; // number of elements (coils or registers) to read
  telegram.u16reg = mbIoTDataBuffer; // pointer to a memory array in the Arduino
  for (int i = 0; i < SUPPORT_UPDATE_REGISTERS; i++) {
    mbIoTDataBuffer[i] = SysSettings[i];
  }
  ModbusQuery(&MBIoTH, telegram);
}

static int IsPowerOffTime(void)
{
  struct MYTIME d;
  const uint8_t powerOffHour = SysSettings[R54_NBIOT_POWEROFF_TIME] / 100;
  const uint8_t powerOffMinute = SysSettings[R54_NBIOT_POWEROFF_TIME] % 100;

  getSysMyTime(&d);
  if ((d.hour == powerOffHour) && (d.minute == powerOffMinute)) {
    return 1;
  }
  return 0;
}

void modbusIoTProc(void) {
  if (SysSettings[R36_RUN_MODE] != MODE_RUNNING) {
      upState = UPLOAD_OPERATION_INIT;
      return;
  }

  if (upData.waitMinuteCounter != getTimeMinuteCount()) {
    upData.waitMinuteCounter = getTimeMinuteCount();
    if (IsPowerOffTime()) {
      upData.waitSecondCounter = getTimeCount();
      upState = TURN_OFF_POWER;
    }
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
    upState = WAIT_UPDATA;
    upData.waitSecondCounter = getTimeCount();
    updateDataToNBIOT();
    break;

  case TEST_UPLOAD_DATA:
    upState = WAIT_UPDATA;
    upData.waitSecondCounter = getTimeCount();
    updateDataToNBIOT();
    break;

  case WAIT_UPDATA:
    if (MBIoTH.i8state == COM_IDLE) {
      ulTaskNotifyTake(pdTRUE, 1000); // for clean notify
      if (MBIoTH.i8lastError != 0)
        DEBUG_PRINTF("Update to NBIOT Gateway failed\n");

      upState = UPLOAD_IDLE_DATA;
      upData.waitSecondCounter = getTimeCount();
    }
    break;

  case TURN_OFF_POWER:
    if ((getTimeCount() - upData.waitSecondCounter) >= TURNOFF_TIME) {
      upState = FIRST_UPDATE;
      NBIOT_ON_CTRL = TURN_ON_NBIOT;
    }
    break;
  } /* end of switch(upState) */
}

void InitMbIoT(void)
{
  memset(mbIoTDataBuffer, 0, sizeof(mbIoTDataBuffer));
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
  MBIoTH.u16regsize = NBIOT_MAX_REGISTERS;
  MBIoTH.u8regsmask = NULL;
  //Initialize Modbus library
  ModbusInit(&MBIoTH);
  //Start capturing traffic on serial Port
  ModbusStart(&MBIoTH);
  upState = UPLOAD_OPERATION_INIT;
  upData.waitSecondCounter = 0;
  upData.waitMinuteCounter = 0;
}
