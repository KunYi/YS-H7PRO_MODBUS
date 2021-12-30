
#include "mytime.h"

uint32_t datetime_since_epoch(const struct MYTIME *mytime)
{
  /* ref. https://howardhinnant.github.io/date_algorithms.html#days_from_civil */
    int32_t year =  mytime->year;
    if(mytime->month <= 2)
       year -= 1;
    const uint32_t era = year / 400;
    const uint32_t yoe = year - era * 400;      // [0, 399]
    const uint32_t doy = (153*(mytime->month + (mytime->month > 2 ? -3 : 9)) + 2)/5 + mytime->day-1;  // [0, 365]
    const uint32_t doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]
    return ((((era * 146097 + doe - 737730) // 737730 mean Jan. 1 2020, 719468 mean Jan. 1 1970
              ) * 24 + mytime->hour
            ) * 60 + mytime->minute
          ) * 60 + mytime->second;
}

void epoch_to_datetime(int32_t epoch, struct MYTIME *mytime)
{
    /* ref. https://howardhinnant.github.io/date_algorithms.html#civil_from_days */
    mytime->second = (epoch % 60);
    const uint32_t se = epoch / 60;
    mytime->minute = (se) % 60;
    const uint32_t me = (se / 60);
    mytime->hour = (me % 24);
    epoch = me/24;
    epoch += 737730; // 737730 mean Jan. 1 2020, 719468 mean Jan. 1 1970
    const int32_t era = (epoch >= 0 ? epoch : epoch - 146096) / 146097;
    const uint32_t doe = (uint32_t)(epoch - era * 146097);          // [0, 146096]
    const uint32_t yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;  // [0, 399]
    const int32_t y = (int32_t)(yoe) + era * 400;
    const uint32_t doy = doe - (365*yoe + yoe/4 - yoe/100);                // [0, 365]
    const uint32_t mp = (5*doy + 2)/153;                                   // [0, 11]
    mytime->day = doy - (153*mp+2)/5 + 1;                             // [1, 31]
    mytime->month = mp < 10 ? mp+3 : mp-9;                              // [1, 12]
    mytime->year = y + ((mytime->month <= 2) ? 1 : 0);
}
