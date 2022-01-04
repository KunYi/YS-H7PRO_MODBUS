
#ifndef _SYS_SETTINGS_H
#define _SYS_SETTINGS_H

#include <stdint.h>

enum OffsetMBSetting {
    R00_TOWER_ID = 0,
    R01_TEMP1_IN = 1,
    R02_TEMP1_OUT = 2,
    R03_EC = 3,
    R04_PH = 4,
    R05_TEMP = 5,
    R06_HUMIDITY = 6,
    R07_TEMP2_IN = 7,
    R08_TEMP2_OUT = 8,
    R09_TEMP3_IN = 9,
    R10_TEMP3_OUT = 10,
    R11_TEMP4_IN = 11,
    R12_TEMP4_OUT = 12,
    R13_TEMP5_IN = 13,
    R14_TEMP5_OUT = 14,
    R15_TEMP6_IN = 15,
    R16_TEMP6_OUT = 16,
    R17_TEMP7_IN = 17,
    R18_TEMP7_OUT = 18,
    R19_TEMP8_IN = 19,
    R20_TEMP8_OUT = 20,
    R21_DRAIN_TIME = 21,
    R22_OP_TOWER_NUM = 22,
    R23_AVG_VOLTAGE = 23,
    R24_AVG_CURRENT = 24,
    R25_INST_POWER = 25,
    R26_CUM_POWER = 26,
    R27_DIGITAL_INPUT = 27,
    R28_SYS_TIME_RL = 28,
    R29_SYS_TIME_RH = 29,
    R30_MODE_STATUS = 30,
    R31_TOWER_MASK = 31,
    R46_SYS_TIME_WL = 46,
    R47_SYS_TIME_WH = 47,
    R48_MANUAL_CMD = 48,
};

enum RUN_MODE {
    MODE_MANUAL = 0,
    MODE_RUNNING = 0xEA55,
};

struct  SystemSettings {
    uint16_t    towerID;                /* R00, Read only */
    uint16_t    temp1In;                /* R01, Read only */
    uint16_t    temp1Out;               /* R02, Read only */
    uint16_t    EC;                     /* R03, Read only */
    uint16_t    PH;                     /* R04, Read only */
    uint16_t    temp;                   /* R05, Read only */
    uint16_t    humidity;               /* R06, Read only */
    uint16_t    inTemp2;                /* R07, Read only */
    uint16_t    outTemp2;               /* R08, Read only */
    uint16_t    inTemp3;                /* R09, Read only */
    uint16_t    outTemp3;               /* R10, Read only */
    uint16_t    inTemp4;                /* R11, Read only */
    uint16_t    outTemp4;               /* R12, Read only */
    uint16_t    inTemp5;                /* R13, Read only */
    uint16_t    outTemp5;               /* R14, Read only */
    uint16_t    inTemp6;                /* R15, Read only */
    uint16_t    outTemp6;               /* R16, Read only */
    uint16_t    inTemp7;                /* R17, Read only */
    uint16_t    outTemp7;               /* R18, Read only */
    uint16_t    inTemp8;                /* R19, Read only */
    uint16_t    outTemp8;               /* R20, Read only */
    uint16_t    drainTime;              /* R21, Read only, lessthen 30000 */
    uint16_t    operatingTowerNum;      /* R22, Read only */
    uint16_t    averageVoltage;         /* R23, Read only */
    uint16_t    averageCurrent;         /* R24, Read only */
    uint16_t    instantaneousPower;     /* R25, Read only */
    uint16_t    cumulativePower;        /* R26, Read only */
    uint16_t    towerRunningTimer;      /* R26, Read only */
    uint16_t    digitalInput;           /* R27, Read only */
    uint16_t    myTimeLow;              /* R28, Read only */
    uint16_t    myTimeHigh;             /* R29, Read only */
    /* the below register is writable */
    uint16_t    runningFlags;           /* R30, automatic/manual */
    uint16_t    towersEnabled;          /* R31, bit 0 ~ bit 7 to mapping tower 1 ~ 8*/
    uint16_t    valvaSwithTimer;        /* R32, must less 300 sec*/
    uint16_t    noWaterTimer;           /* R33, must less 300 sec*/
    uint16_t    pumpCountDownTimer;     /* R34 */
    uint16_t    tower1SettingTimer;     /* R35 */
    uint16_t    tower2SettingTimer;     /* R36 */
    uint16_t    tower3SettingTimer;     /* R37 */
    uint16_t    tower4SettingTimer;     /* R38 */
    uint16_t    tower5SettingTimer;     /* R39 */
    uint16_t    tower6SettingTimer;     /* R40 */
    uint16_t    tower7SettingTimer;     /* R41 */
    uint16_t    tower8SettingTimer;     /* R42 */
    uint16_t    triggerEC;              /* R43 */
    uint16_t    triggerPH;              /* R44 */
    uint16_t    cleaningTimer;          /* R45 */
    uint16_t    myTimeLowW;             /* R46, Write only */
    uint16_t    myTimeHighW;            /* R47, Write only */
    uint16_t    manualCmd;              /* R48, Write only */
};

#endif /* End of _SYS_SETTINGS_H */
