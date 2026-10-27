/* localtime - libc16 (docs/spec/time.md) */

#include <time.h>
#include "time16.h"

static struct tm result;

struct tm * __cdecl localtime(const time_t *t)
{
    if (!__libc16_tzdone)
        tzset();
    __libc16_civil(*t, -timezone, &result);
    result.tm_isdst = daylight && __libc16_isdst(&result);
    if (result.tm_isdst)
        __libc16_civil(*t, 3600 - timezone, &result);
    return &result;
}
