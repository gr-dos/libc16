/* Calendar arithmetic for the time and file members - libc16
 * (docs/spec/time.md) */

#include <time.h>
#include "time16.h"

/* Days before each month in a year that is not a leap year */
static const int before[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

long __libc16_year_start(int y)
{
    long days = 0;
    int i;

    if (y >= 1970) {
        for (i = 1970; i < y; i++)
            days += YEAR_DAYS(i);
    } else {
        for (i = y; i < 1970; i++)
            days -= YEAR_DAYS(i);
    }
    return days;
}

int __libc16_yday(int y, int mon, int mday)
{
    return before[mon] + (mon >= 2 && YEAR_DAYS(y) == 366) + mday - 1;
}

void __libc16_civil(long t, long adjust, struct tm *tm)
{
    long days = t / 86400;
    long secs = t % 86400 + adjust;
    int y = 1970, mon, leap;
    unsigned s;

    while (secs < 0) {
        secs += 86400;
        days--;
    }
    while (secs >= 86400) {
        secs -= 86400;
        days++;
    }
    s = (unsigned)(secs / 60);
    tm->tm_sec = (int)(secs - s * 60L);
    tm->tm_min = s % 60;
    tm->tm_hour = s / 60;
    /* 1970-01-01 was a Thursday */
    tm->tm_wday = (int)((days % 7 + 11) % 7);
    while (days < 0) {
        y--;
        days += YEAR_DAYS(y);
    }
    while (days >= YEAR_DAYS(y)) {
        days -= YEAR_DAYS(y);
        y++;
    }
    tm->tm_year = y - 1900;
    tm->tm_yday = (int)days;
    leap = YEAR_DAYS(y) == 366;
    for (mon = 11; tm->tm_yday < before[mon] + (mon >= 2 && leap); mon--)
        ;
    tm->tm_mon = mon;
    tm->tm_mday = tm->tm_yday - before[mon] - (mon >= 2 && leap) + 1;
}

/* Day of the year of the first Sunday on or after day yday of year y */
static int sunday(int y, int yday)
{
    long day = __libc16_year_start(y) + yday;

    /* (day % 7 + 11) % 7 is the weekday, 0 for Sunday */
    return yday + (7 - (int)((day % 7 + 11) % 7)) % 7;
}

int __libc16_isdst(const struct tm *tm)
{
    int y = tm->tm_year + 1900;
    int leap = YEAR_DAYS(y) == 366;
    int start = sunday(y, 59 + leap + 7);       /* second Sunday in March */
    int end = sunday(y, 304 + leap);            /* first Sunday in November */

    /* From 2:00 standard time on the start day to 2:00 daylight time,
     * 1:00 standard, on the end day */
    return (tm->tm_yday > start || (tm->tm_yday == start && tm->tm_hour >= 2))
        && (tm->tm_yday < end || (tm->tm_yday == end && tm->tm_hour < 1));
}

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
