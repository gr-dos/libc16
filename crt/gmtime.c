/* gmtime - libc16 (docs/spec/time.md) */

#include <time.h>
#include "time16.h"

/* Its own result, apart from localtime's, so that gmtime does not link
 * localtime (and through it tzset and getenv) */
static struct tm result;

struct tm * __cdecl gmtime(const time_t *t)
{
    __libc16_civil(*t, 0, &result);
    result.tm_isdst = 0;
    return &result;
}
