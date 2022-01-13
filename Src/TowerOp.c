#include <stddef.h>
#include <string.h>
#include "debug.h"
#include "sysSettings.h"
#include "swTimer.h"
#include "TowerOp.h"
#include "sysIO.h"

enum TOWER_STATE_MACHINE {
  TOWER_OPERATION_INIT,
  SELECT_TOWER,
  ENABLE_TOWER,
  CHECK_PUMP_ON,
  CHECK_FLOW_ACTIVE,
  WAIT_PUMP_COOLDOWN,
  TOWER_RUNNING,
  ERROR_STOP,
};

struct TowerOperationData {
  uint32_t  waitCounter;           /* for waiting counter */
  uint8_t   retryCounter;          /* for retry no water flow */
  int8_t    currentTower;          /* for current operation tower */
  int8_t    continueTowerFailed;
};

enum TOWER_STATE_MACHINE towerState;
static struct TowerOperationData towerData;

uint32_t dbgCount = 0;
#define SIMULATION_IO_AUTO  1

static int isEnabledTowerX(uint8_t num)
{
  return ((SysSettings[R37_TOWER_ENABLED] & (1 << num)) != 0);
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
  if (SysSettings[R36_RUN_MODE] != MODE_RUNNING) {
      towerState = TOWER_OPERATION_INIT;
      return;
  }

  switch(towerState) {
  case TOWER_OPERATION_INIT:
    DEBUG_PRINTF("Init Tower Operation\n");
    towerState = SELECT_TOWER;
    memset(&towerData, 0, sizeof(struct TowerOperationData));
    towerData.currentTower = -1;
    SysSettings[R30_PUMP_NOWATER] = 0;
    SysSettings[R31_PUMP_COOLDOWN] = 0;
    SysSettings[R23_OP_MINUTE] = 0;
    SysSettings[R22_OP_TOWER_NUM] = 0;

  case SELECT_TOWER:
    dbgCount = 0;
    towerData.currentTower = getNextTower(towerData.currentTower);
    if (towerData.currentTower >= 0) {
      DEBUG_PRINTF("SELECT_TOWER towerId:%d, TRANSTION to ENALE_TOWER\n", towerData.currentTower);
      towerData.retryCounter  = 0;
      towerState = ENABLE_TOWER;
    } else {
      towerState = TOWER_OPERATION_INIT;
    }
    break;

  case ENABLE_TOWER:
      DEBUG_PRINTF ("ENABLE_TOWER, TRANSTION to CHECK_PUMP_ON\n");
      towerState = CHECK_PUMP_ON;
      sysTurnOnTower(towerData.currentTower);
      PUMP_ON_CTRL = TURN_ON_PUMP;

      SysSettings[R30_PUMP_NOWATER] = 0;
      SysSettings[R31_PUMP_COOLDOWN] = 0;
      SysSettings[R23_OP_MINUTE] = 0;
      SysSettings[R22_OP_TOWER_NUM] =  (towerData.currentTower + 1);

      dbgCount = 0;
    break;

  case CHECK_PUMP_ON:
    // DEBUG_PRINTF("CHK_PUMP_ON towerId:%d\n", towerData.currentTower);
    if (PUMP_STATUS == PUMP_ACTIVE_STATUS) {
      towerState = CHECK_FLOW_ACTIVE;
      towerData.waitCounter = getTimeCount(); /* for Pump Protect */
    }
    #ifdef SIMULATION_IO_AUTO
    if (dbgCount++ > 200) {
      DEBUG_PRINTF("Simulation to CHECK_FLOW_ACTIVE\n");
      towerState = CHECK_FLOW_ACTIVE;
      dbgCount = 0;
      towerData.waitCounter = getTimeCount();
    }
    #endif
    break;

  case CHECK_FLOW_ACTIVE:
    /* check water flow status */
    if (FLOW_STATUS == FLOW_ACTIVE_STATUS) {
      DEBUG_PRINTF("TRANSTION to WAIT_PUMP_COOLDOWN\n");
      towerState = TOWER_RUNNING;
      towerData.continueTowerFailed = 0;
      towerData.waitCounter = getTimeCount(); // for running time
    }
    /* check no water flow over pumpNoWaterTime */
    if (SysSettings[R30_PUMP_NOWATER] != (getTimeCount() - towerData.waitCounter)) {
      SysSettings[R30_PUMP_NOWATER] = (getTimeCount() - towerData.waitCounter);
      DEBUG_PRINTF("CHECK_FLOW_ACTIVE towerId:%d\n", towerData.currentTower);
      DEBUG_PRINTF("No Water, how long:%d sec\n", SysSettings[R30_PUMP_NOWATER]);
    }
    SysSettings[R30_PUMP_NOWATER] = (getTimeCount() - towerData.waitCounter);
    if (SysSettings[R30_PUMP_NOWATER] >= SysSettings[R46_PUMP_NOWATER_TIME]) {
      DEBUG_PRINTF("TRANSTION to WAIT_PUMP_COOLDOWN\n");
      towerState = WAIT_PUMP_COOLDOWN;
      PUMP_ON_CTRL = TURN_OFF_PUMP;
      towerData.waitCounter = getTimeCount(); /* for Pump cool down */
    }
    #ifdef SIMULATION_IO_AUTO
    if (dbgCount++ > 3000) {
      DEBUG_PRINTF("Simulation to TOWER_RUNNING\n");
      towerState = TOWER_RUNNING;
      dbgCount = 0;
      towerData.waitCounter = getTimeCount();
    }
    #endif
    break;

  case WAIT_PUMP_COOLDOWN:
    if (SysSettings[R31_PUMP_COOLDOWN] != (getTimeCount() - towerData.waitCounter)) {
      SysSettings[R31_PUMP_COOLDOWN] = getTimeCount() - towerData.waitCounter;
      DEBUG_PRINTF("WAIT_PUMP_COOLDOWN, how long:%d sec\n", SysSettings[R31_PUMP_COOLDOWN]);
    }

    if (SysSettings[R31_PUMP_COOLDOWN] >= SysSettings[R47_PUMP_COOLDOWN_TIME]) {
      if (TOWER_MAX_RETRY > ++towerData.retryCounter)
      {
        DEBUG_PRINTF("WAIT_PUMP_COOLDOWN, retry:%d, TRANSTION to ENABLE_TOWER\n", towerData.retryCounter);
        towerState = ENABLE_TOWER;
      }
      else {
        if (MAX_FAILED_CONTINUE_TOWER > ++towerData.continueTowerFailed) {
          DEBUG_PRINTF("WAIT_PUMP_COOLDOWN, TRANSTION to SELECT_TOWER, continueTowerFailed: %d\n", towerData.continueTowerFailed);
          towerState = SELECT_TOWER;
        }
        else {
          DEBUG_PRINTF("WAIT_PUMP_COOLDOWN, TRANSTION to ERROR_STOP, continueTowerFailed: %d\n", towerData.continueTowerFailed);
          towerState = ERROR_STOP;
        }
      }
    }
    break;

  case TOWER_RUNNING:
    if (SysSettings[R23_OP_MINUTE] != (getTimeCount() - towerData.waitCounter) / 60) {
      SysSettings[R23_OP_MINUTE] = (getTimeCount() - towerData.waitCounter) / 60;
      DEBUG_PRINTF("TOWER_RUNNING, how long:%d minute\n", SysSettings[R23_OP_MINUTE]);
    }

    if (SysSettings[R23_OP_MINUTE] >= SysSettings[R38_TOWER1_TIME + towerData.currentTower]) {
      towerState = SELECT_TOWER;
      DEBUG_PRINTF("Finish TOWER_RUNNING, TRASNSTION to SELECT_TOWER\n");
      PUMP_ON_CTRL = TURN_OFF_PUMP;
      sysTurnOffTower(towerData.currentTower);
    }
    break;

  case ERROR_STOP:
    break;
  } /* end of switch(towerState) */

}
