/* time - libc16 (docs/spec/time.md) */

#include <time.h>
#include "time16.h"

/* INT 21h 2Ah: the year in the high word, month << 8 | day in the low */
static unsigned long dos_date(void);
#pragma aux dos_date = \
    "mov ah, 2ah" \
    "int 21h" \
    value [cx dx] modify exact [ax cx dx];

/* INT 21h 2Ch: hour << 8 | minute in the high word, second << 8 |
 * hundredths in the low */
static unsigned long dos_time(void);
#pragma aux dos_time = \
    "mov ah, 2ch" \
    "int 21h" \
    value [cx dx] modify exact [ax cx dx];

time_t __cdecl time(time_t *t)
{
    unsigned long date = dos_date();
    unsigned long now = dos_time();
    unsigned long secs;
    unsigned long date2 = dos_date();
    struct tm tm;
    long adjust;
    int y, hour;
    time_t r;

    /* Past midnight between the calls: the time again, with the new date */
    if (date2 != date) {
        date = date2;
        now = dos_time();
    }
    y = (int)(date >> 16);
    hour = (int)(now >> 24);
    tm.tm_yday = __libc16_yday(y, ((unsigned)date >> 8) - 1, (unsigned char)date);

    if (!__libc16_tzdone)
        tzset();
    adjust = timezone;
    /* In daylight saving time the wall clock is an hour ahead of standard
     * time: is the time an hour earlier inside it? An hour of -1 is the
     * previous day's last, as far as __libc16_isdst can tell. In the hour
     * that repeats in November this takes daylight time. */
    if (daylight) {
        tm.tm_year = y - 1900;
        tm.tm_hour = hour - 1;
        if (__libc16_isdst(&tm))
            adjust -= 3600;
    }
    /* Unsigned, since DOS dates reach 2099: at most about 4.1e9 */
    secs = (unsigned long)(__libc16_year_start(y) + tm.tm_yday) * 86400UL
        + (unsigned long)hour * 3600 + (unsigned)((unsigned char)(now >> 16) * 60)
        + (unsigned char)((unsigned)now >> 8) + (unsigned long)adjust;
    r = secs > 0x7FFFFFFFUL ? (time_t)-1 : (time_t)secs;
    if (t != NULL)
        *t = r;
    return r;
}
