
#ifndef __CFG_H__
#define __CFG_H__

#include <stdint.h>
#include <string.h>

struct CFG {
    uint32_t  magicMark;                            /* 4 */
    uint32_t  seqID;                                /* 8 */
    uint32_t  dirtyMark;                            /* 12 */
    uint16_t  TowerID;                              /* 14 */
    uint16_t  runMode;                              /* 16 */
    uint16_t  TowerEnabled;                         /* 18 */
    uint16_t  TowerTime[8];                         /* 34 */
    uint16_t  pumpNoWaterTime;                      /* 36 */
    uint16_t  pumpCoolDownTime;
    uint16_t  triggerEC;
    uint16_t  triggerPH;
    uint16_t  cleaningTime;
    uint16_t  cleaningCoolDownTime;
    uint16_t  nbIoTProcTime;
    uint16_t  nbIoTPowerOffTime;
    uint16_t  sourceFlow;
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


#define CFG_ADDRESS         ((uint32_t) 0x08020000)    /* SECTOR 1 */
#endif /* end of file __CFG_H__ */
