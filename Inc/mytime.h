
#ifndef _MY_TIME_H
#define _MY_TIME_H

#include <stdint.h>
#include <stdio.h>

struct MYTIME {
    int32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
};

uint32_t datetime_since_epoch(const struct MYTIME *mytime);
void epoch_to_datetime(int32_t epoch, struct MYTIME *mytime);

#endif /* End of _MY_TIME_H */
