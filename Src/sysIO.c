#include "sysIO.h"

union INPUT sysIn;
union OUTPUT sysOut;

void initSysIO() {
    sysIn._in = 0;
    sysOut._out = 0;
}

void sysSetOutput(uint8_t num)
{
    sysOut._out = 1<<num;
}
