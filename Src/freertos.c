/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "rtc.h"
#include "SEGGER_RTT.h"
#include "mytime.h"
#include "sysSettings.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myMbReadSensors */
osThreadId_t myMbReadSensorsHandle;
const osThreadAttr_t myMbReadSensors_attributes = {
  .name = "myMbReadSensors",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myMbSettingsTas */
osThreadId_t myMbSettingsTasHandle;
const osThreadAttr_t myMbSettingsTas_attributes = {
  .name = "myMbSettingsTas",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myMbIoTTask */
osThreadId_t myMbIoTTaskHandle;
const osThreadAttr_t myMbIoTTask_attributes = {
  .name = "myMbIoTTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for basic1SecTimer */
osTimerId_t basic1SecTimerHandle;
const osTimerAttr_t basic1SecTimer_attributes = {
  .name = "basic1SecTimer"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern void InitMbReadSensors(void);
extern void InitMbSettings(void);
extern void InitMbIoT(void);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
extern void StartMbReadSensorsTask(void *argument);
extern void StartMbSettingsTask(void *argument);
extern void StartMbIoTTask(void *argument);
extern void basic1SecCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  InitMbReadSensors();
  InitMbSettings();
  InitMbIoT();


  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of basic1SecTimer */
  basic1SecTimerHandle = osTimerNew(basic1SecCallback, osTimerPeriodic, NULL, &basic1SecTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  osTimerStart(basic1SecTimerHandle, 1000U); /* start Timer 1 second */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of myMbReadSensors */
  myMbReadSensorsHandle = osThreadNew(StartMbReadSensorsTask, NULL, &myMbReadSensors_attributes);

  /* creation of myMbSettingsTas */
  myMbSettingsTasHandle = osThreadNew(StartMbSettingsTask, NULL, &myMbSettingsTas_attributes);

  /* creation of myMbIoTTask */
  myMbIoTTaskHandle = osThreadNew(StartMbIoTTask, NULL, &myMbIoTTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */


extern struct SystemSettings*  pSysSettings;

void basic1SecCallback(void *argument)
{
  static uint32_t count = 0;

  count++;
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
  pSysSettings->myTimeLow = (uint16_t)(epoch & 0xFFFF);
  pSysSettings->myTimeHigh = (uint16_t)(epoch >> 16);

  if ((count % 10) == 0) {
    SEGGER_RTT_printf(0, "current date:time %0.2d/%0.2d/%0.4d, ", sdate.Month, sdate.Date, sdate.Year + 2000);
    SEGGER_RTT_printf(0, "%0.2d:%0.2d:%0.2d \n", stime.Hours, stime.Minutes, stime.Seconds);
    #if 0
    SEGGER_RTT_printf(0, "epoch:%d, high16:%d, low16%d\n", epoch, pSysSettings->myTimeHigh, pSysSettings->myTimeLow);
    struct MYTIME mytime={ .year = 2021, .month = 12, .day = 30, .hour = 20, .minute = 1, .second=32 };
    int32_t time =  datetime_since_epoch(&mytime);
    SEGGER_RTT_printf(0, "%d\n", time);
    struct MYTIME test={0};
    epoch_to_datetime(time, &test);
    SEGGER_RTT_printf(0, "%0.2d/%0.2d/%0.4d, %0.2d:%0.2d:%0.2d\n", test.month, test.day, test.year, test.hour, test.minute, test.second);
    #endif
  }

}
/* USER CODE END Application */
