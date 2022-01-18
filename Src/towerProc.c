#include <stddef.h>
#include <string.h>
#include "debug.h"
#include "sysSettings.h"
#include "swTimer.h"
#include "towerProc.h"
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
static uint8_t isRunning;

uint32_t dbgCount = 0;
#define SIMULATION_FLOW_ON        1
#define SIMULATION_PUMP_ON        1

static int isEnabledTowerX(uint8_t num)
{
  return ((SysStatus[R37_TOWER_ENABLED] & (1 << num)) != 0);
}

static int8_t getNextTower(uint8_t n) {
    if (++n >= 7) n = 0;

    for (int8_t i = n; i < 8; i++) {
        if (isEnabledTowerX(i))
            return i;
    }
    return -1;
}

static void initOutput(void)
{
    VALVE_ON_CTRL = TURN_OFF_VALVE;
    PUMP_ON_CTRL = TURN_OFF_PUMP;
    ELECTRODE_ON_CTRL = TURN_OFF_ELECTRODE;
    for (int i = 0; i < 8; i++)
      sysTurnOffTower(i);
}

void initTowerProc(void) {
  isRunning = 0;
  towerState = TOWER_OPERATION_INIT;
  initOutput();
}

void TowerProc(void)
{
  if (SysStatus[R36_RUN_MODE] != MODE_RUNNING) {
      towerState = TOWER_OPERATION_INIT;
      SysStatus[R22_OP_TOWER_NUM] = 0;
      SysStatus[R23_OP_MINUTE] = 0;
      if (isRunning) {
        initOutput();
        isRunning = 0;
      }
      return;
  }

  switch(towerState) {
  case TOWER_OPERATION_INIT:
    DEBUG_PRINTF("Init Tower Operation\n");
    towerState = SELECT_TOWER;
    memset(&towerData, 0, sizeof(struct TowerOperationData));
    towerData.currentTower = -1;
    SysStatus[R22_OP_TOWER_NUM] = 0;
    SysStatus[R23_OP_MINUTE] = 0;
    SysStatus[R30_PUMP_NOWATER] = 0;
    SysStatus[R31_PUMP_COOLDOWN] = 0;
    isRunning = 1;
    initOutput();

  case SELECT_TOWER:
    dbgCount = 0;
    towerData.currentTower = getNextTower(towerData.currentTower);
    if (towerData.currentTower >= 0) {
      DEBUG_PRINTF("SELECT_TOWER towerId:%d, TRANSTION to ENABLE_TOWER\n", towerData.currentTower);
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

      SysStatus[R30_PUMP_NOWATER] = 0;
      SysStatus[R31_PUMP_COOLDOWN] = 0;
      SysStatus[R23_OP_MINUTE] = 0;
      SysStatus[R22_OP_TOWER_NUM] =  (towerData.currentTower + 1);

      dbgCount = 0;
    break;

  case CHECK_PUMP_ON:
    // DEBUG_PRINTF("CHK_PUMP_ON towerId:%d\n", towerData.currentTower);
    if (PUMP_STATUS == PUMP_ACTIVE_STATUS) {
      towerState = CHECK_FLOW_ACTIVE;
      towerData.waitCounter = getTimeCount(); /* for Pump Protect */
    }
    #ifdef SIMULATION_PUMP_ON
    if (dbgCount++ > 500) {
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
    if (SysStatus[R30_PUMP_NOWATER] != (getTimeCount() - towerData.waitCounter)) {
      SysStatus[R30_PUMP_NOWATER] = (getTimeCount() - towerData.waitCounter);
      DEBUG_PRINTF("CHECK_FLOW_ACTIVE towerId:%d\n", towerData.currentTower);
      DEBUG_PRINTF("No Water, how long:%d sec\n", SysStatus[R30_PUMP_NOWATER]);
    }
    SysStatus[R30_PUMP_NOWATER] = (getTimeCount() - towerData.waitCounter);
    if (SysStatus[R30_PUMP_NOWATER] >= SysStatus[R46_PUMP_NOWATER_TIME]) {
      DEBUG_PRINTF("TRANSTION to WAIT_PUMP_COOLDOWN\n");
      towerState = WAIT_PUMP_COOLDOWN;
      PUMP_ON_CTRL = TURN_OFF_PUMP;
      towerData.waitCounter = getTimeCount(); /* for Pump cool down */
    }
    #ifdef SIMULATION_FLOW_ON
    if (dbgCount++ > 500) {
      DEBUG_PRINTF("Simulation to TOWER_RUNNING\n");
      towerState = TOWER_RUNNING;
      dbgCount = 0;
      towerData.waitCounter = getTimeCount();
    }
    #endif
    break;

  case WAIT_PUMP_COOLDOWN:
    if (SysStatus[R31_PUMP_COOLDOWN] != (getTimeCount() - towerData.waitCounter)) {
      SysStatus[R31_PUMP_COOLDOWN] = getTimeCount() - towerData.waitCounter;
      DEBUG_PRINTF("WAIT_PUMP_COOLDOWN, how long:%d sec\n", SysStatus[R31_PUMP_COOLDOWN]);
    }

    if (SysStatus[R31_PUMP_COOLDOWN] >= SysStatus[R47_PUMP_COOLDOWN_TIME]) {
      if (TOWER_MAX_RETRY > ++towerData.retryCounter)
      {
        DEBUG_PRINTF("WAIT_PUMP_COOLDOWN, retry:%d, TRANSTION to ENABLE_TOWER\n", towerData.retryCounter);
        towerState = ENABLE_TOWER;
        sysTurnOffTower(towerData.currentTower);
      }
      else {
        if (MAX_FAILED_CONTINUE_TOWER > ++towerData.continueTowerFailed) {
          DEBUG_PRINTF("WAIT_PUMP_COOLDOWN, TRANSTION to SELECT_TOWER, continueTowerFailed: %d\n", towerData.continueTowerFailed);
          towerState = SELECT_TOWER;
          PUMP_ON_CTRL = TURN_OFF_PUMP;
          sysTurnOffTower(towerData.currentTower);
        }
        else {
          DEBUG_PRINTF("WAIT_PUMP_COOLDOWN, TRANSTION to ERROR_STOP, continueTowerFailed: %d\n", towerData.continueTowerFailed);
          towerState = ERROR_STOP;
        }
      }
    }
    break;

  case TOWER_RUNNING:
    if (SysStatus[R23_OP_MINUTE] != (getTimeCount() - towerData.waitCounter) / 60) {
      SysStatus[R23_OP_MINUTE] = (getTimeCount() - towerData.waitCounter) / 60;
      DEBUG_PRINTF("TOWER_RUNNING, how long:%d minute\n", SysStatus[R23_OP_MINUTE]);
    }

    if (SysStatus[R23_OP_MINUTE] >= SysStatus[R38_TOWER1_TIME + towerData.currentTower]) {
      towerState = SELECT_TOWER;
      DEBUG_PRINTF("Finish TOWER_RUNNING, TRASNSTION to SELECT_TOWER\n");
      PUMP_ON_CTRL = TURN_OFF_PUMP;
      sysTurnOffTower(towerData.currentTower);
    }
    break;

  case ERROR_STOP:
    initOutput();
    ELECTRODE_ON_CTRL = TURN_ON_ELECTRODE;
    SysStatus[R22_OP_TOWER_NUM] = 9;
    break;
  } /* end of switch(towerState) */

}
