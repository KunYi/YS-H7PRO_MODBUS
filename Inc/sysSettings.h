
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
    R21_TOTAL_DRAIN_TIME = 21,
    R22_OP_TOWER_NUM = 22,
    R23_OP_MINUTE = 23,                 /* for display tower operation time */
    R24_DIGITAL_INPUT = 24,             /* Digital Input */
    R25_DIGITAL_OUTPUT = 25,
    R26_RESERVE = 26,
    R27_DRAIN_TIME = 27,
    R28_CLEANING_COOLDOWN = 28,
    R29_VALVE_SWITCH_TIME = 29,
    R30_PUMP_NOWATER = 30,              /* for display no flow when pump active */
    R31_PUMP_COOLDOWN = 31,             /* for display cool down of pump */
    R32_SOURCE_FLOW = 32,
    R33_NBIOT_PROC = 33,
    R34_SYS_TIME_RL = 34,
    R35_SYS_TIME_RH = 35,
    R36_RUN_MODE = 36,                  /* for switch run/settings */
    R37_TOWER_ENABLED = 37,             /* for select tower range */
    R38_TOWER1_TIME = 38,               /* operation time setting for tower 1 */
    R39_TOWER2_TIME = 39,               /* operation time setting for tower 2 */
    R40_TOWER3_TIME = 40,               /* operation time setting for tower 3 */
    R41_TOWER4_TIME = 41,               /* operation time setting for tower 4 */
    R42_TOWER5_TIME = 42,               /* operation time setting for tower 5 */
    R43_TOWER6_TIME = 43,               /* operation time setting for tower 6 */
    R44_TOWER7_TIME = 44,               /* operation time setting for tower 7 */
    R45_TOWER8_TIME = 45,               /* operation time setting for tower 8 */
    R46_PUMP_NOWATER_TIME = 46,         /* protect time setting when enable pump but no flow */
    R47_PUMP_COOLDOWN_TIME = 47,        /* cool down time setting for pump */
    R48_EC_TRIGGER = 48,                /* EC condition setting for trigger */
    R49_PH_TRIGGER = 49,
    R50_CLEANING_TIME = 50,
    R51_CLEANING_COOLDOWN_TIME = 51,
    R52_SOURCE_FLOW_TIME = 52,
    R53_NBIOT_TIME = 53,
    R54_NBIOT_POWEROFF_TIME = 54,
    R55_FIELD_MASK = 55,
    R56_SYS_TIME_WL = 56,
    R57_SYS_TIME_WH = 57,
    R58_MANUAL_CMD = 58,
    MAX_SETTINGS_REG = 59
};

enum RUN_MODE {
    MODE_MANUAL = 0,
    MODE_RUNNING = 0xEA55,
};

#define DI_MASK_FLOW    (0x0001)
#define DI_MASK_PUMP    (0x0002)

#if defined ( __CC_ARM )
#define KEIL_PACKED __attribute__((packed))
#define GCC_PACKED
#else
#define KEIL_PACKED
#define GCC_PACKED __attribute__((packed))
#endif

#if 0
struct  SystemSettings {
    uint16_t    towerID;                /* R00, Write/Read */
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
    uint16_t    totalDrainTime;         /* R21, Read only, lessthen 30000, DO0  */
    uint16_t    operatingTowerNum;      /* R22, Read only */
    uint16_t    towerOpMinute;          /* R23, Read only */
    uint16_t    digitalInput;           /* R24, Read only */
    uint16_t    reserve1;               /* R25, Read only */
    uint16_t    reserve2;               /* R26, Read only */
    uint16_t    drainTime;              /* R27, Read only */
    uint16_t    cleaningCoolDown;       /* R28, Read only */
    uint16_t    valveSwitchTime;        /* R29, must less 300 sec*/
    uint16_t    pumpNoWater;            /* R30, must less 300 sec*/
    uint16_t    pumpCoolDown;           /* R31, must less 6000 sec*/
    uint16_t    sourceFlow;             /* R32, Read only */
    uint16_t    nbIoTProc;              /* R33, Read only */
    uint16_t    myTimeLow;              /* R34, Read only */
    uint16_t    myTimeHigh;             /* R35, Read only */
    /* the below register is writable */
    uint16_t    runMode;                /* R36, automatic/manual */
    uint16_t    towersEnabled;          /* R37, bit 0 ~ bit 7 to mapping tower 1 ~ 8*/
    uint16_t    tower1SettingTime;      /* R38 */
    uint16_t    tower2SettingTime;      /* R39 */
    uint16_t    tower3SettingTime;      /* R40 */
    uint16_t    tower4SettingTime;      /* R41 */
    uint16_t    tower5SettingTime;      /* R42 */
    uint16_t    tower6SettingTime;      /* R43 */
    uint16_t    tower7SettingTime;      /* R44 */
    uint16_t    tower8SettingTime;      /* R45 */
    uint16_t    pumpNoWaterTime;        /* R46, for Pump Protect when no water flow, the pump can't run over the time */
    uint16_t    pumpCoolDownTime;       /* R47, for Pump Cool Down */
    uint16_t    triggerEC;              /* R48 */
    uint16_t    triggerPH;              /* R49 */
    uint16_t    cleaningTime;           /* R50 */
    uint16_t    cleaningCoolDownTime;   /* R51 */
    uint16_t    sourceFlowTime;         /* R52 */
    uint16_t    nbIoTProcTime;          /* R53 */
    uint16_t    nbIoTPowerOffTime;      /* R54 */
    uint16_t    fieldMask;              /* R55 */
    uint16_t    myTimeLowW;             /* R56, Write only */
    uint16_t    myTimeHighW;            /* R57, Write only */
    uint16_t    manualCmd;              /* R58, Write only */
} GCC_PACKED ;
#endif

extern volatile uint16_t SysStatus[MAX_SETTINGS_REG];
#endif /* End of _SYS_SETTINGS_H */
