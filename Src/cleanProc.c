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

enum CLEAN_STATE_MACHINE cleanState;

static int IsTrigger(void)
{
    if (SysSettings[R03_EC] >= SysSettings[R48_EC_TRIGGER])
    {
        return 1;
    }

    if (SysSettings[R49_PH_TRIGGER] > 1400)
        return 0;

    if (SysSettings[R49_PH_TRIGGER] < 700) {
        if (SysSettings[R03_EC] <= SysSettings[R49_PH_TRIGGER]) {
            return 1;
        }
    }
    else {
        if (SysSettings[R03_EC] >= SysSettings[R49_PH_TRIGGER]) {
            return 1;
        }
    }
    return 0;
}

void initCleanProc(void)
{
     cleanState = CLEAN_OPERATION_INIT;
}

void CleanProc(void)
{
    if (SysSettings[R36_RUN_MODE] != MODE_RUNNING) {
        cleanState = CLEAN_OPERATION_INIT;
        return;
    }

    switch (cleanState) {
    case CLEAN_OPERATION_INIT:
        break;
    case CHECK_WATER_QUALITY:
        if (IsTrigger()) {

        }
        break;
    case CLEAN_RUNNING:
        break;
    case WAIT_CLEAN_COOLDOWN:
        break;
    }
}
