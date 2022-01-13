
#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "SEGGER_RTT.h"
#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED 0 /* default disable, please to setting in project or makefile */
#endif
#ifndef INFO_ENABLED
#define INFO_ENABLED  1
#endif

#define DEBUG_PRINTF(fmt, ...) \
            do { if (DEBUG_ENABLED) SEGGER_RTT_printf(0, fmt, ## __VA_ARGS__); \
            } while(0)

#define INFO_PRINTF(fmt, ...) \
            do { if (INFO_ENABLED) SEGGER_RTT_printf(0, fmt, ## __VA_ARGS__); \
            } while(0)

#endif /* __DEBUG_H_ */
