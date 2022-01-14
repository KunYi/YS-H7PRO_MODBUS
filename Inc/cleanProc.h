#ifndef _CLEAN_PROC_H_
#define _CLEAN_PROC_H_

#include <stdint.h>

void initCleanProc(void);
void CleanProc(void);

uint32_t getCumulateDrainTime(void);
void resetCumulateDrainTime(void);

#endif /* end of file, _CLEAN_PROC_H_ */
