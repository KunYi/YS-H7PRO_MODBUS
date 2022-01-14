
#ifndef __DEFAULT_CONF_H
#define __DEFAULT_CONF_H

#define MODBUS_SETTINGS_UART   huart3 /* USB to COM, for settings*/
#define MODBUS_MASTER_UART     huart4 /* RS485 Port 1 for Sensor*/
#define MODBUS_IOT_UART        huart8 /* RS485 Port 2, PE0, PE1 UART8 for NB-IOT*/

#define MODBUS_SLAVE_ID	       (1)
#define TOWER_ID               (12345)

/* for NBIOT */
#define UPLOAD_PERIOD          (15 * 60)    /* 15 minutes   */
#define WAIT_FIRST_UPLOAD      (3*60 + 30)  /* 3.5 minutes  */
#define TURNOFF_TIME           (30)         /* 30 seoncd    */

/* BCD format */
/* default 01/20/2022, 19:30:30 */
#define DEFAULT_TIME_HOUR      (0x19)
#define DEFAULT_TIME_MINUTE    (0x30)
#define DEFAULT_TIME_SECOND    (0x30)
#define DEFAULT_WEEKDAY        (RTC_WEEKDAY_THURSDAY)
#define DEFAULT_DATE_YEAR      (0x22)
#define DEFAULT_DATE_MONTH     (RTC_MONTH_JANUARY)
#define DEFAULT_DATE_DAY       (0x20)

/* sensor's slave ID */
#define SENSOR_EC              (9)
#define SENSOR_PH              (8)
#define SENSOR_TEMP1           (3)
#define SENSOR_TEMP2           (4)

#define MB_DIO                 (20)
#endif /* end of __DEFAULT_CONF_H */
