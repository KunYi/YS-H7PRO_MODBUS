
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include <stdint.h>
#include "keyProc.h"

void keyProc(void) {
    uint32_t tick = osKernelSysTick();
}
