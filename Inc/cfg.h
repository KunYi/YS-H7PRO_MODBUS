
#ifndef __CFG_H__
#define __CFG_H__

#include <stdint.h>
#include <string.h>

struct CFG {
    uint32_t  magicMark;                            /* 4 */
    uint32_t  seqID;                                /* 8 */
    uint32_t  dirtyMark;                            /* 12 */
    uint16_t  towerID;                              /* 14 */
    uint16_t  runMode;                              /* 16 */
    uint16_t  towersEnabled;                        /* 18 */
    uint16_t  towerTime[8];                         /* 34 */
    uint16_t  valveSwitchTime;                      /* 36 */
    uint16_t  pumpNoWaterTime;                      /* 38 */
    uint16_t  pumpCoolDownTime;                     /* 40 */
    uint16_t  triggerEC;                            /* 42 */
    uint16_t  triggerPH;                            /* 44 */
    uint16_t  cleaningTime;                         /* 46 */
    uint16_t  cleaningCoolDownTime;                 /* 48 */
    uint16_t  nbIoTProcTime;                        /* 50 */
    uint16_t  nbIoTPowerOffTime;                    /* 52 */
    uint16_t  sourceFlowTime;                       /* 54 */
    uint16_t  fieldMask;                            /* 56 */
};

#define CFG_MAGIC   ((uint32_t)0xAB1255AA)
#define INIT_CFG(x) do \
                    { \
                        memset((x), 0, sizeof(struct CFG)); \
                        (x)->magicMark = CFG_MAGIC; \
                        (x)->dirtyMark = ~((uint32_t)0UL);  \
                    } while(0)

void readCFG(struct CFG* cfg);
void saveCFG(struct CFG* cfg);
void saveInitCFG(void);

void checkAndLoadCfg(void);
void updateAndSaveCfg(void);

#define CFG_ADDRESS         ((uint32_t) 0x08020000)    /* SECTOR 1 */
#define CFG_SECTOR          (FLASH_SECTOR_1)
#endif /* end of file __CFG_H__ */
