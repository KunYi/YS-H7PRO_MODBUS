#include <stddef.h>
#include "debug.h"
#include "sysSettings.h"
#include "swTimer.h"
#include "TowerOp.h"
#include "sysIO.h"

enum TowerOpMachine {
    TOWER_OP_INIT,
    SEL_TOWER,
    CHK_PUMP_ON,
    CHK_FLOW_ACTIVE,
};

enum TowerOpMachine tOpManchine;
int8_t curTower;
int8_t towerFailed;
uint32_t dbgCount = 0;

static int isEnabledTowerX(uint8_t num)
{
  return ((pSysSettings->towersEnabled & 1<<num) != 0);
}

static int8_t getNextTower(uint8_t n) {
    if (++n >= 7) n = 0;

    for (int8_t i = n; i < 8; i++) {
        if (isEnabledTowerX(i))
            return i;
    }
    return -1;
}

void TowerProc(void)
{
  if (pSysSettings->runMode != MODE_RUNNING) {
      tOpManchine = TOWER_OP_INIT;
      towerFailed = 0;
      return;
  }

  switch(tOpManchine) {
  case TOWER_OP_INIT:
    DEBUG_PRINTF("INIT TowerOperation\n");
    tOpManchine = SEL_TOWER;
    curTower = -1;
  case SEL_TOWER:
    curTower = getNextTower(curTower);
    dbgCount = 0;
    if (curTower >= 0) {
        DEBUG_PRINTF("SEL_TOWER towerId:%d\n", curTower);
        sysSetOutput(curTower + DO4);
        sysSetOutput(DO2); // Pump On
        tOpManchine = CHK_PUMP_ON;
    } else {
        tOpManchine = TOWER_OP_INIT;
    }
    break;
  case CHK_PUMP_ON:
    DEBUG_PRINTF("CHK_PUMP_ON towerId:%d\n", curTower);
    if (sysIn.in.X1) {
        tOpManchine = CHK_FLOW_ACTIVE;
    }
    if (dbgCount++ > 200) {
        tOpManchine = SEL_TOWER;
    }
    break;
  case CHK_FLOW_ACTIVE:
    DEBUG_PRINTF("CHK_FLOW_ACTIVE towerId:%d\n", curTower);
    if (sysIn.in.X0) {
    }
    break;
  }
}
