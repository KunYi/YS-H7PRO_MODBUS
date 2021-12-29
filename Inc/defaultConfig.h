
#ifndef __DEFAULT_CONF_H
#define __DEFAULT_CONF_H

#define MODBUS_SETTINGS_UART   huart3 /* USB to COM, for settings*/
#define MODBUS_MASTER_UART     huart4 /* RS485 Port 1 for Sensor*/
#define MODBUS_IOT_UART        huart8 /* RS485 Port 2, PE0, PE1 UART8 for NB-IOT*/

#define MODBUS_SLAVE_ID	       (1)
#define TOWER_ID               (12345)

/* BCD format */
/* default 12/29/2021, 19:30:30 */
#define DEFAULT_TIME_HOUR      (0x19)
#define DEFAULT_TIME_MINUTE    (0x30)
#define DEFAULT_TIME_SECOND    (0x30)
#define DEFAULT_WEEKDAY        (RTC_WEEKDAY_WEDNESDAY)
#define DEFAULT_DATE_YEAR      (0x21)
#define DEFAULT_DATE_MONTH     (RTC_MONTH_DECEMBER)
#define DEFAULT_DATE_DAY       (0x29)

#endif /* end of __DEFAULT_CONF_H */
