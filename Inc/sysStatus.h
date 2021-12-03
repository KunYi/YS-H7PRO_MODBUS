
#ifndef _SYS_STATUS_H
#define _SYS_STATUS_H

#include <stdint.h>

struct  SystemStatus {
    uint16_t    TowerNo;                /* R00 */
    uint16_t    TempIn1;                /* R01 */
    uint16_t    TempOutput1;            /* R02 */
    uint16_t    EC;                     /* R03 */
    uint16_t    PH;                     /* R04 */
    uint16_t    AirTemp;                /* R05 */
    uint16_t    AirHumidity;            /* R06 */
    uint16_t    TempIn2;                /* R07 */
    uint16_t    TempOutput2;            /* R08 */
    uint16_t    TempIn3;                /* R09 */
    uint16_t    TempOutput3;            /* R10 */
    uint16_t    TempIn4;                /* R11 */
    uint16_t    TempOutput4;            /* R12 */
    uint16_t    TempIn5;                /* R13 */
    uint16_t    TempOutput5;            /* R14 */
    uint16_t    TempIn6;                /* R15 */
    uint16_t    TempOutput6;            /* R16 */
    uint16_t    TempIn7;                /* R17 */
    uint16_t    TempOutput7;            /* R18 */
    uint16_t    TempIn8;                /* R19 */
    uint16_t    TempOutput8;            /* R20 */
    uint16_t    DrainMinutes;           /* R21 */
    uint16_t    FlowSwitch;             /* R22 */
    uint16_t    Voltage;                /* R23, for Meter */
    uint16_t    Current;                /* R24 */
    uint16_t    InstantaneousPower;     /* R25 */
    uint16_t    CumulativePower;        /* R26 */
    uint16_t    CumulativePower1;       /* R27 */
    uint16_t    CumulativeFlowTon;      /* R28, for Water meter */
    uint16_t    CumulativeFlowLiter;    /* R29 */
    uint16_t    StatusLine;             /* R30 */
    uint16_t    ControlLine;            /* R31 */
    uint16_t    VoltageOutput;          /* R32 */
    uint16_t    CurrentOutput;          /* R33 */
    uint16_t    PowerOutput;            /* R34 */
    uint16_t    VoltageSet;             /* R35 */
    uint16_t    CurrentSet;             /* R36 */
    uint16_t    FrequenceSet;           /* R37 */
    uint16_t    WorkStatus;             /* R38 */
    uint16_t    OverHeat;               /* R39 */
    uint16_t    OverCurent;             /* R40 */
    uint16_t    WaterShortage;          /* R41 */
    uint16_t    OverVoltage;            /* R42 */
    uint16_t    UnderVoltage;           /* R43 */
    uint16_t    PhaseFailure;           /* R44 */
    uint16_t    OtherFailure;           /* R45 */
    uint16_t    Failure;                /* R46 */
    uint16_t    EmergencyStop;          /* R47 */
};

#endif  /* End of _SYS_STATUS_H */
