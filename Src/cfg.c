
#include "main.h"
#include "cfg.h"
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
    defCfg.TowerID = 12345;
    defCfg.runMode = 0; // manaul mode
    defCfg.TowerEnabled = 0xF; // default Tower 1 ~ Tower 4 is enabled
    for (int i = 0; i < 8; i++) {
        defCfg.TowerTime[i] = 180; // default 180 minutes
    }
    defCfg.pumpNoWaterTime = 30; // 30 seconds
    defCfg.pumpCoolDownTime = 60; // 60 seconds
    defCfg.triggerEC = 1500;
    defCfg.triggerPH = 1500; // disable PH
    defCfg.cleaningTime = 30; // 30 seconds
    defCfg.cleaningCoolDownTime = 600; // 600 seconds
    saveCFG(&defCfg);
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
        return;
    }

    uint32_t flash_addr = CFG_ADDRESS;
    memset(flashbuff, 0xFF, 32);
    memcpy (flashbuff, cfg, 32);
    /* write 32bytes, ref. STM32H743 ref. manual, minimal unit write is 256bits */
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, flash_addr, (uint32_t)flashbuff) != HAL_OK)
    {
        DEBUG_PRINTF("FAILED! HAL_FLASH_Program\n");
        return;
    }
    flash_addr += 32;
    memset(flashbuff, 0xFF, 32);
    memcpy(flashbuff, ((uint8_t*)cfg) + 32, sizeof(struct CFG) - 32);
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, flash_addr, (uint32_t)flashbuff) != HAL_OK) // write 32bytes
    {
        DEBUG_PRINTF("FAILED! HAL_FLASH_Program\n");
        return;
    }

    HAL_FLASH_Lock();
}
