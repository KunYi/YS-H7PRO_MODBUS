
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "main.h"

#include <stdint.h>
#include <string.h>
#include "debug.h"
#include "keyProc.h"

#define GPIO_KEY_DOWN   GPIO_PIN_RESET
#define GPIO_KEY_RELEASE GPIO_PIN_SET
#define KEY_DEBOUNCE_TIME   (20UL)
#define KEY_LONG_PRESS_TIME (500UL)

enum {
    RELEASE,
    DEBOUNCE,
    PRESSED,
    LONG_PRESSED,
};

struct KEY_STATE {
    uint32_t downTime;
    uint8_t state;
};

static struct KEY_STATE keyState[4] = { 0 };
void gpioKeyProc(struct KEY_STATE *key, GPIO_TypeDef *GPIOx , uint16_t pin)
{

    if (GPIO_KEY_DOWN == HAL_GPIO_ReadPin(GPIOx, pin)) {

        switch(key->state) {
        case RELEASE:
            key->downTime = osKernelSysTick();
            key->state = DEBOUNCE;
            break;
        case DEBOUNCE:
            if (( (osKernelSysTick() - key->downTime) > osKernelSysTickMicroSec(KEY_DEBOUNCE_TIME * 1000UL))) {
                key->state = PRESSED;
            }
            break;
        case PRESSED:
            if (( (osKernelSysTick() - key->downTime) > osKernelSysTickMicroSec(KEY_LONG_PRESS_TIME * 1000UL))) {
                key->state = LONG_PRESSED;
            }
            break;
        case LONG_PRESSED:
            break;
        }
    }
    else {
        switch(key->state) {
        case LONG_PRESSED:
        case PRESSED:
        case DEBOUNCE:
            key->state = RELEASE;
            break;
        }
    }
}

void keyProc(void) {
    gpioKeyProc(&keyState[0], KEY1_GPIO_Port, KEY1_Pin);
    gpioKeyProc(&keyState[1], KEY2_GPIO_Port, KEY2_Pin);
    gpioKeyProc(&keyState[2], KEY3_GPIO_Port, KEY3_Pin);
    gpioKeyProc(&keyState[3], KEY4_GPIO_Port, KEY4_Pin);
}

void initKey(void) {
    memset(keyState, 0, sizeof(keyState));
}
