#include "sysIO.h"

volatile union INPUT sysIn;
volatile union OUTPUT sysOut;

void initSysIO() {
    sysIn._in = 0;
    sysOut._out = 0;
}

void sysSetOutput(uint8_t num)
{
    sysOut._out |= (1 << num);
}

void sysTurnOnTower(uint8_t num)
{
    sysOut._out |= (1 << (num + NUM_DO4));
}

void sysTurnOffTower(uint8_t num)
{
    sysOut._out &= ~((uint16_t)(1 << (num + NUM_DO4)));
}
