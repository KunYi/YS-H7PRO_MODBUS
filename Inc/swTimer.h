#ifndef _SW_TIMER_H_
#define _SW_TIMER_H_

#include <stddef.h>

void initSwTimer(void);
uint32_t getTimeCount(void);
uint32_t getTimeMinuteCount(void);

void basic1SecCallback(void *argument);

#endif /* end of file, _SW_TIMER_H_ */
