#include <stddef.h>
#include <string.h>
#include "debug.h"
#include "sysSettings.h"
#include "swTimer.h"
#include "cleanProc.h"
#include "sysIO.h"

enum CLEAN_STATE_MACHINE {
  CLEAN_OPERATION_INIT,
  CHECK_WATER_QUALITY,
  CLEAN_RUNNING,
  WAIT_CLEAN_COOLDOWN,
};

struct CleanOperationData {
  uint32_t  waitCounter;           /* for waiting counter */
  volatile uint32_t  cumulateTime;
};

enum CLEAN_STATE_MACHINE cleanState;
static struct CleanOperationData cleanData;

uint32_t getCumulateDrainTime() {
    return cleanData.cumulateTime;
}

void resetCumulateDrainTime() {
    cleanData.cumulateTime = 0;
}

static int IsTrigger(void)
{
    if (SysStatus[R03_EC] >= SysStatus[R48_EC_TRIGGER])
    {
        return 1;
    }

    if (SysStatus[R49_PH_TRIGGER] > 1400)
        return 0;

    if (SysStatus[R49_PH_TRIGGER] < 700) {
        if (SysStatus[R03_EC] <= SysStatus[R49_PH_TRIGGER]) {
            return 1;
        }
    }
    else {
        if (SysStatus[R03_EC] >= SysStatus[R49_PH_TRIGGER]) {
            return 1;
        }
    }
    return 0;
}

void initCleanProc(void)
{
     cleanState = CLEAN_OPERATION_INIT;
     cleanData.cumulateTime = 0;
}

void CleanProc(void)
{
    if (SysStatus[R36_RUN_MODE] != MODE_RUNNING) {
        cleanState = CLEAN_OPERATION_INIT;
        return;
    }

    switch (cleanState) {
    case CLEAN_OPERATION_INIT:
        DEBUG_PRINTF("CLEAN_OPERATION_INIT, TRANSTION to CHECK_WATER_QUALITY\n");
        cleanState = CHECK_WATER_QUALITY;
        SysStatus[R27_DRAIN_TIME] = 0;
        SysStatus[R28_CLEANING_COOLDOWN] = 0;

    case CHECK_WATER_QUALITY:
        if (IsTrigger()) {
            VALVE_ON_CTRL = TURN_ON_VALVE;
            DEBUG_PRINTF("CHECK_WATER_QUALITY, TRANSTION to CLEAN_RUNNING\n");
            cleanState = CLEAN_RUNNING;
            cleanData.waitCounter = getTimeCount();
        }
        break;

    case CLEAN_RUNNING:
        if (SysStatus[R27_DRAIN_TIME] != (getTimeCount() - cleanData.waitCounter)) {
            SysStatus[R27_DRAIN_TIME] = getTimeCount() - cleanData.waitCounter;
            DEBUG_PRINTF("CLEAN_RUNNING:%d, running time:%d\n", SysStatus[R27_DRAIN_TIME]);
        }
        if (SysStatus[R27_DRAIN_TIME] >= SysStatus[R50_CLEANING_TIME])  {
            DEBUG_PRINTF("CLEAN_RUNNING, TRANSTION to WAIT_CLEAN_COOLDOWN\n");
            cleanState = WAIT_CLEAN_COOLDOWN;
            cleanData.waitCounter = getTimeCount();
            cleanData.cumulateTime += SysStatus[R27_DRAIN_TIME];
            SysStatus[R29_VALVE_SWITCH_TIME] = getCumulateDrainTime() / 60;
            SysStatus[R21_TOTAL_DRAIN_TIME] = getCumulateDrainTime() / 60;
            VALVE_ON_CTRL = TURN_OFF_VALVE;
        }
        break;

    case WAIT_CLEAN_COOLDOWN:
        if (SysStatus[R28_CLEANING_COOLDOWN] != (getTimeCount() - cleanData.waitCounter)) {
            SysStatus[R28_CLEANING_COOLDOWN] = getTimeCount() - cleanData.waitCounter;
            DEBUG_PRINTF("WAIT_CLEAN_COOLDOWN:%d, cooldown time:%d\n", SysStatus[R28_CLEANING_COOLDOWN]);
        }
        if (SysStatus[R28_CLEANING_COOLDOWN] > SysStatus[R51_CLEANING_COOLDOWN_TIME]) {
            DEBUG_PRINTF("Finish WAIT_CLEAN_COOLDOWN, TRANSTION to CHECK_WATER_QUALITY\n");
            cleanState = CHECK_WATER_QUALITY;
            SysStatus[R27_DRAIN_TIME] = 0;
            SysStatus[R28_CLEANING_COOLDOWN] = 0;
        }
        break;

    } /* end of switch(cleanState) */
}
