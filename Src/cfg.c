
#include "main.h"
#include "cfg.h"
#include "sysSettings.h"
#include "defaultConfig.h"
#include "debug.h"

static uint8_t flashbuff[32];
void readCFG(struct CFG* cfg)
{
    memcpy(cfg, (void *)CFG_ADDRESS, sizeof(struct CFG));
}

void saveInitCFG(void)
{
    struct CFG  defCfg;
    INIT_CFG(&defCfg);
    defCfg.towerID = TOWER_ID;
    defCfg.runMode = 0; // manaul mode
    defCfg.towersEnabled = 0xF; // default Tower 1 ~ Tower 4 is enabled
    for (int i = 0; i < 8; i++) {
        defCfg.towerTime[i] = 180; // default 180 minutes
    }
    defCfg.valveSwitchTime = 10; // 10 seconds
    defCfg.pumpNoWaterTime = 30; // 30 seconds
    defCfg.pumpCoolDownTime = 60; // 60 seconds
    defCfg.triggerEC = 1500;
    defCfg.triggerPH = 1500; // disable PH
    defCfg.cleaningTime = 30; // 30 seconds
    defCfg.cleaningCoolDownTime = 600; // 600 seconds
    defCfg.nbIoTProcTime = 9;   // 9 seconds
    defCfg.nbIoTPowerOffTime = 0;
    defCfg.sourceFlowTime = 900;    // 900 seconds
    defCfg.fieldMask = 0;
    saveCFG(&defCfg);
}

static void initSysSettings(void) {
  for (int i = 0; i < MAX_SETTINGS_REG; i++)
	  SysSettings[i] = 0;
}

void checkAndLoadCfg(void)
{
  struct CFG sysCfg;

  initSysSettings();
  readCFG(&sysCfg);
  // for default value when config is empty
  if (sysCfg.magicMark != CFG_MAGIC) {
      saveInitCFG();
      readCFG(&sysCfg);
      if (sysCfg.magicMark != CFG_MAGIC) {
        DEBUG_PRINTF("CFG init failed\n");
      }
  }

  // restore to SysSettings
  SysSettings[R00_TOWER_ID] = sysCfg.towerID;
  SysSettings[R36_RUN_MODE] = sysCfg.runMode;
  SysSettings[R37_TOWER_ENABLED] = sysCfg.towersEnabled;

  for (int i = 0; i < 8; i++) {
    SysSettings[R38_TOWER1_TIME + i] = sysCfg.towerTime[i];
  }

  SysSettings[R29_VALVE_SWITCH_TIME] = sysCfg.valveSwitchTime;
  SysSettings[R46_PUMP_NOWATER_TIME] = sysCfg.pumpNoWaterTime;
  SysSettings[R47_PUMP_COOLDOWN_TIME] = sysCfg.pumpCoolDownTime;
  SysSettings[R48_EC_TRIGGER] = sysCfg.triggerEC;
  SysSettings[R49_PH_TRIGGER] = sysCfg.triggerPH;
  SysSettings[R50_CLEANING_TIME] = sysCfg.cleaningTime;
  SysSettings[R51_CLEANING_COOLDOWN_TIME] = sysCfg.cleaningCoolDownTime;
  SysSettings[R52_SOURCE_FLOW_TIME] = sysCfg.sourceFlowTime;
  SysSettings[R53_NBIOT_TIME] = sysCfg.nbIoTProcTime;
  SysSettings[R54_NBIOT_POWEROFF_TIME] = sysCfg.nbIoTPowerOffTime;
  SysSettings[R55_FIELD_MASK] = sysCfg.fieldMask;
}

void updateAndSaveCfg(void)
{
  struct CFG sysCfg;
  INIT_CFG(&sysCfg);

  sysCfg.towerID = SysSettings[R00_TOWER_ID];
  sysCfg.runMode = SysSettings[R36_RUN_MODE];
  sysCfg.towersEnabled =  SysSettings[R37_TOWER_ENABLED];

  for (int i = 0; i < 8; i++) {
     sysCfg.towerTime[i] = SysSettings[R38_TOWER1_TIME + i];
  }

  sysCfg.valveSwitchTime = SysSettings[R29_VALVE_SWITCH_TIME];
  sysCfg.pumpNoWaterTime = SysSettings[R46_PUMP_NOWATER_TIME];
  sysCfg.pumpCoolDownTime = SysSettings[R47_PUMP_COOLDOWN_TIME];
  sysCfg.triggerEC = SysSettings[R48_EC_TRIGGER];
  sysCfg.triggerPH = SysSettings[R49_PH_TRIGGER];
  sysCfg.cleaningTime = SysSettings[R50_CLEANING_TIME];
  sysCfg.cleaningCoolDownTime = SysSettings[R51_CLEANING_COOLDOWN_TIME];
  sysCfg.sourceFlowTime = SysSettings[R52_SOURCE_FLOW_TIME];
  sysCfg.nbIoTProcTime = SysSettings[R53_NBIOT_TIME];
  sysCfg.nbIoTPowerOffTime = SysSettings[R54_NBIOT_POWEROFF_TIME];
  sysCfg.fieldMask = SysSettings[R55_FIELD_MASK];
  saveCFG(&sysCfg);
}

void saveCFG(struct CFG *cfg)
{
    uint32_t ret;
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef erase = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3, /* write 32bits once */
        .Banks = 1,
        .Sector = CFG_SECTOR,
        .NbSectors = 1
    };

    if (HAL_FLASHEx_Erase(&erase, &ret) != HAL_OK)
    {
        DEBUG_PRINTF("FAILED HAL_FLASHEx_Erase\n");
        HAL_FLASH_Lock();
        return;
    }

    uint32_t flash_addr = CFG_ADDRESS;
    memset(flashbuff, 0xFF, 32);
    memcpy (flashbuff, cfg, 32);
    /* write 32bytes, ref. STM32H743 ref. manual, minimal unit write is 256bits/32bytes */
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, flash_addr, (uint32_t)flashbuff) != HAL_OK)
    {
        DEBUG_PRINTF("FAILED! HAL_FLASH_Program\n");
        HAL_FLASH_Lock();
        return;
    }
    flash_addr += 32;
    memset(flashbuff, 0xFF, 32);
    memcpy(flashbuff, ((uint8_t*)cfg) + 32, sizeof(struct CFG) - 32);
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, flash_addr, (uint32_t)flashbuff) != HAL_OK) // write 32bytes
    {
        DEBUG_PRINTF("FAILED! HAL_FLASH_Program\n");
        HAL_FLASH_Lock();
        return;
    }

    HAL_FLASH_Lock();
}
