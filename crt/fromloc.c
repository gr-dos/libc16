/* Local wall-clock time to time_t - libc16 (docs/spec/time.md,
 * docs/spec/files.md). Its own member, apart from the calendar arithmetic
 * in timecal, because it calls tzset: gmtime links timecal but must not
 * link tzset. */

#include <time.h>
#include "time16.h"

time_t __libc16_from_local(long local)
{
    struct tm tm;

    if (!__libc16_tzdone)
        tzset();
    /* A wall-clock time in daylight saving time is an hour ahead of
     * standard time; in the hour that repeats in November, the daylight
     * one is taken. */
    if (daylight) {
        __libc16_civil(local, -3600, &tm);
        if (__libc16_isdst(&tm))
            return local - 3600 + timezone;
    }
    return local + timezone;
}
