
#ifndef _SYS_SETTINGS_H
#define _SYS_SETTINGS_H

#include <stdint.h>

struct  SystemSettings {
    uint16_t    digitalInput;               /* R00 */
    uint16_t    tower1RunningTimer;
    uint16_t    tower2RunningTimer;
    uint16_t    tower3RunningTimer;
    uint16_t    tower4RunningTimer;
    uint16_t    tower5RunningTimer;
    uint16_t    tower6RunningTimer;
    uint16_t    tower7RunningTimer;
    uint16_t    tower8RunningTimer;
    uint16_t    inTemp1;
    uint16_t    outTemp1;
    uint16_t    inTemp2;
    uint16_t    outTemp2;
    uint16_t    inTemp3;
    uint16_t    outTemp3;
    uint16_t    inTemp4;
    uint16_t    outTemp4;
    uint16_t    inTemp5;
    uint16_t    outTemp5;
    uint16_t    inTemp6;
    uint16_t    outTemp6;
    uint16_t    inTemp7;
    uint16_t    outTemp7;
    uint16_t    inTemp8;
    uint16_t    outTemp8;
    uint16_t    EC;
    uint16_t    PH;                         /* R26 */

    /* the below register is writable */
    uint16_t    towerID;
    uint16_t    myTimeLow;
    uint16_t    myTimeHigh;
    uint16_t    timeHigh;
    uint16_t    runningFlags;        /* automatic/manual */
    uint16_t    towersEnabled;       /* bit 0 ~ bit 7 to mapping tower 1 ~ 8*/
    uint16_t    valvaSwithTimer;     /* must less 300 sec*/
    uint16_t    noWaterTimer;        /* must less 300 sec*/
    uint16_t    pumpCountDownTimer;
    uint16_t    tower1SettingTimer;
    uint16_t    tower2SettingTimer;
    uint16_t    tower3SettingTimer;
    uint16_t    tower4SettingTimer;
    uint16_t    tower5SettingTimer;
    uint16_t    tower6SettingTimer;
    uint16_t    tower7SettingTimer;
    uint16_t    tower8SettingTimer;
    uint16_t    triggerEC;
    uint16_t    triggerPH;
    uint16_t    cleaningTimer; /* 清洗等待 */
};

#endif /* End of _SYS_SETTINGS_H */
