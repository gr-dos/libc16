/* gmtime (docs/spec/time.md). Expected fields from the host's gmtime. Run
 * with TZ unset and with EST5EDT, which must not change the results. It
 * calls nothing else, so its members show that gmtime links neither
 * tzset nor localtime; tests/time/clock checks that gmtime and localtime
 * have separate results. */

#include <time.h>
#include "check.h"

struct gcase {
    time_t t;
    struct tm tm;
};

/* sec, min, hour, mday, mon, year, wday, yday, isdst */
static const struct gcase cases[] = {
    { 0L, { 0, 0, 0, 1, 0, 70, 4, 0, 0 } },
    { 951782400L, { 0, 0, 0, 29, 1, 100, 2, 59, 0 } },
    { 951868799L, { 59, 59, 23, 29, 1, 100, 2, 59, 0 } },
    { 978307199L, { 59, 59, 23, 31, 11, 100, 0, 365, 0 } },
    { 1234567890L, { 30, 31, 23, 13, 1, 109, 5, 43, 0 } },
    { 2147483647L, { 7, 14, 3, 19, 0, 138, 2, 18, 0 } },
    { -1L, { 59, 59, 23, 31, 11, 69, 3, 364, 0 } },
    { -86400L, { 0, 0, 0, 31, 11, 69, 3, 364, 0 } },
    { -2147483647L - 1, { 52, 45, 20, 13, 11, 1, 5, 346, 0 } },
    { 68169600L, { 0, 0, 0, 29, 1, 72, 2, 59, 0 } },
    { 1709251199L, { 59, 59, 23, 29, 1, 124, 4, 59, 0 } },
    { 1960058751L, { 51, 45, 20, 10, 1, 132, 2, 40, 0 } },
};

#define NCASES (sizeof cases / sizeof cases[0])

static int same(const struct tm *a, const struct tm *b)
{
    return a->tm_sec == b->tm_sec && a->tm_min == b->tm_min && a->tm_hour == b->tm_hour
        && a->tm_mday == b->tm_mday && a->tm_mon == b->tm_mon && a->tm_year == b->tm_year
        && a->tm_wday == b->tm_wday && a->tm_yday == b->tm_yday && a->tm_isdst == b->tm_isdst;
}

int main(void)
{
    struct tm *g, *g2;
    time_t t;
    unsigned i;

    for (i = 0; i < NCASES; i++) {
        g = gmtime(&cases[i].t);
        if (g == NULL || !same(g, &cases[i].tm)) {
            t_puts("not ok case ");
            t_putu(i);
            t_puts("\r\n");
            failures++;
        }
    }

    /* the same static object every time */
    t = 0;
    g = gmtime(&t);
    t = 86400L;
    g2 = gmtime(&t);
    CHECK(g == g2 && g->tm_mday == 2);

    return done();
}
