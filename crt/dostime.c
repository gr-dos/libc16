/* DOS file dates and times to and from time_t - libc16
 * (docs/spec/files.md) */

#include <time.h>
#include "time16.h"

time_t __libc16_from_dos(unsigned date, unsigned time)
{
    int y = 1980 + (date >> 9);
    int mon = ((date >> 5) & 15) - 1;
    int mday = date & 31;
    long local;

    /* DOS does not check what it stores; clamp what it cannot mean. */
    if (mon < 0)
        mon = 0;
    else if (mon > 11)
        mon = 11;
    if (mday < 1)
        mday = 1;
    local = (__libc16_year_start(y) + __libc16_yday(y, mon, mday)) * 86400
        + (long)(time >> 11) * 3600 + ((time >> 5) & 63) * 60 + (time & 31) * 2;
    return __libc16_from_local(local);
}

int __libc16_to_dos(time_t t, unsigned *date, unsigned *time)
{
    struct tm *tm = localtime(&t);

    if (tm->tm_year < 80 || tm->tm_year > 207)
        return -1;
    *date = (tm->tm_year - 80) << 9 | (tm->tm_mon + 1) << 5 | tm->tm_mday;
    *time = tm->tm_hour << 11 | tm->tm_min << 5 | tm->tm_sec / 2;
    return 0;
}
