
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

void checkAndLoadCfg(void)
{
  struct CFG sysCfg;

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
  pSysSettings->towerID = sysCfg.towerID;
  pSysSettings->runMode = sysCfg.runMode;
  pSysSettings->towersEnabled = sysCfg.towersEnabled;

  uint16_t *pPointer = &pSysSettings->tower1SettingTime;
  for (int i = 0; i < 8; i++) {
    *pPointer = sysCfg.towerTime[i];
    pPointer++;
  }
  pSysSettings->valveSwitchTime = sysCfg.valveSwitchTime;
  pSysSettings->pumpNoWaterTime = sysCfg.pumpNoWaterTime;
  pSysSettings->pumpCoolDownTime = sysCfg.pumpCoolDownTime;
  pSysSettings->triggerEC = sysCfg.triggerEC;
  pSysSettings->triggerPH = sysCfg.triggerPH;
  pSysSettings->cleaningTime = sysCfg.cleaningTime;
  pSysSettings->cleaningCoolDownTime = sysCfg.cleaningCoolDownTime;
  pSysSettings->nbIoTProcTime = sysCfg.nbIoTProcTime;
  pSysSettings->nbIoTPowerOffTime = sysCfg.nbIoTPowerOffTime;
  pSysSettings->sourceFlowTime = sysCfg.sourceFlowTime;
  pSysSettings->fieldMask = sysCfg.fieldMask;
}

void updateAndSaveCfg(void)
{
  struct CFG sysCfg;
  INIT_CFG(&sysCfg);

  sysCfg.towerID = pSysSettings->towerID;
  sysCfg.runMode = pSysSettings->runMode;
  sysCfg.towersEnabled =  pSysSettings->towersEnabled;

  uint16_t *pPointer = &pSysSettings->tower1SettingTime;
  for (int i = 0; i < 8; i++) {
     sysCfg.towerTime[i] = *pPointer;
     pPointer++;
  }

  sysCfg.valveSwitchTime = pSysSettings->valveSwitchTime;
  sysCfg.pumpNoWaterTime = pSysSettings->pumpNoWaterTime;
  sysCfg.pumpCoolDownTime = pSysSettings->pumpCoolDownTime;
  sysCfg.triggerEC = pSysSettings->triggerEC;
  sysCfg.triggerPH = pSysSettings->triggerPH;
  sysCfg.cleaningTime = pSysSettings->cleaningTime;
  sysCfg.cleaningCoolDownTime = pSysSettings->cleaningCoolDownTime;
  sysCfg.nbIoTProcTime = pSysSettings->nbIoTProcTime;
  sysCfg.nbIoTPowerOffTime = pSysSettings->nbIoTPowerOffTime;
  sysCfg.sourceFlowTime = pSysSettings->sourceFlowTime;
  sysCfg.fieldMask = pSysSettings->fieldMask;
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
        .Sector = FLASH_SECTOR_1,
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
