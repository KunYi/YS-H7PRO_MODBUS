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

void InitMbIoT(void);
void StartMbIoTTask(void *argument);

static void ModbusIoTProc(void);

void StartMbIoTTask(void *argument)
{
  for(;;) {
    ModbusIoTProc();
    osDelay(1000);
  }
}

static void ModbusIoTProc(void) {

}

void InitMbIoT(void)
{

}
