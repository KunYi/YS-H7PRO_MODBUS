#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include <stdint.h>
#include "rtc.h"
#include "SEGGER_RTT.h"
#include "mytime.h"
#include "swTimer.h"
#include "sysSettings.h"

volatile uint32_t timeCount = 0;

void initSwTimer(void)
{
    timeCount = 0;
}

uint32_t getTimeCount(void)
{
    return timeCount;
}

void basic1SecCallback(void *argument)
{
  timeCount++;
  RTC_DateTypeDef sdate={0};
  RTC_TimeTypeDef stime={0};
  /*
    don't change GetTime & GetDate sequence
    STM32 need keep the operation order for RTC
  */
  HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
  struct MYTIME temp = {
                       .year = sdate.Year + 2000,
                       .month = sdate.Month,
                       .day = sdate.Date,
                       .hour = stime.Hours,
                       .minute = stime.Minutes,
                       .second = stime.Seconds
                      };
  uint32_t epoch = datetime_since_epoch(&temp);
  SysSettings[R34_SYS_TIME_RL] = (uint16_t)(epoch & 0xFFFF);
  SysSettings[R35_SYS_TIME_RH] = (uint16_t)(epoch >> 16);
#if 0
  if ((count % 10) == 0) {
    SEGGER_RTT_printf(0, "current date:time %0.2d/%0.2d/%0.4d, ", sdate.Month, sdate.Date, sdate.Year + 2000);
    SEGGER_RTT_printf(0, "%0.2d:%0.2d:%0.2d \n", stime.Hours, stime.Minutes, stime.Seconds);
    /*
    SEGGER_RTT_printf(0, "epoch:%d, high16:%d, low16%d\n", epoch, pSysSettings->myTimeHigh, pSysSettings->myTimeLow);
    struct MYTIME mytime={ .year = 2021, .month = 12, .day = 30, .hour = 20, .minute = 1, .second=32 };
    int32_t time =  datetime_since_epoch(&mytime);
    SEGGER_RTT_printf(0, "%d\n", time);
    struct MYTIME test={0};
    epoch_to_datetime(time, &test);
    SEGGER_RTT_printf(0, "%0.2d/%0.2d/%0.4d, %0.2d:%0.2d:%0.2d\n", test.month, test.day, test.year, test.hour, test.minute, test.second);
    */
  }
#endif

}
