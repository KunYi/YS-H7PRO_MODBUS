#ifndef _SW_TIMER_H_
#define _SW_TIMER_H_

#include <stddef.h>
#include "mytime.h"

void initSwTimer(void);
uint32_t getTimeCount(void);
uint32_t getTimeMinuteCount(void);
void getSysMyTime(struct MYTIME *);

void basic1SecCallback(void *);

#endif /* end of file, _SW_TIMER_H_ */
