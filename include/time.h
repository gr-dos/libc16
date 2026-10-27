/* time.h - libc16 (C89 7.12, the parts docs/spec/time.md implements, and
 * Microsoft C's tzset, timezone, daylight and tzname) */

#ifndef _LIBC16_TIME_H
#define _LIBC16_TIME_H

#include <_libc16.h>

#ifndef _LIBC16_SIZE_T
#define _LIBC16_SIZE_T
typedef unsigned size_t;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

/* Seconds since 1970-01-01 00:00:00 UTC */
#ifndef _LIBC16_TIME_T
#define _LIBC16_TIME_T
typedef long time_t;
#endif

struct tm {
    int tm_sec;     /* 0 to 60 */
    int tm_min;     /* 0 to 59 */
    int tm_hour;    /* 0 to 23 */
    int tm_mday;    /* 1 to 31 */
    int tm_mon;     /* 0 to 11 */
    int tm_year;    /* years since 1900 */
    int tm_wday;    /* 0 to 6, Sunday is 0 */
    int tm_yday;    /* 0 to 365 */
    int tm_isdst;   /* positive in daylight saving time, 0 if not */
};

/* Set by tzset from TZ: seconds west of UTC, whether a daylight saving
 * zone is named, and the standard and daylight zone names. */
extern long   __cdecl timezone;
extern int    __cdecl daylight;
extern char * __cdecl tzname[2];

struct tm * __cdecl localtime(const time_t *t);
size_t      __cdecl strftime(char *s, size_t max, const char *format, const struct tm *tm);
void        __cdecl tzset(void);

#endif
