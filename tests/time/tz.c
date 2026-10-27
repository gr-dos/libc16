/* tzset and localtime (docs/spec/time.md): prints the zone that TZ gives
 * and the local time of chosen instants, for the expect files */

#include <libc16.h>
#include <time.h>
#include "testio.h"

/* 2026's daylight saving changes for EST5EDT: 2026-03-08 07:00 UTC and
 * 2026-11-01 06:00 UTC */
static const time_t when[] = {
    0, 946684800L, 951782400L, 978307199L, 2147483647L, -1L, -2147483647L - 1,
    1772953199L, 1772953200L, 1793512799L, 1793512800L, 1793516400L
};

static void put_long(long v)
{
    char buf[34];

    t_puts(libc16_ltoa(v, buf, 10));
}

static void put2(int v)
{
    char buf[3];

    buf[0] = (char)('0' + v / 10);
    buf[1] = (char)('0' + v % 10);
    buf[2] = '\0';
    t_puts(buf);
}

int main(void)
{
    struct tm *tm;
    unsigned i;

    tzset();
    t_puts("timezone ");
    put_long(timezone);
    t_puts("\r\ndaylight ");
    put_long(daylight);
    t_puts("\r\ntzname [");
    t_puts(tzname[0]);
    t_puts("] [");
    t_puts(tzname[1]);
    t_puts("]\r\n");
    for (i = 0; i < sizeof when / sizeof when[0]; i++) {
        tm = localtime(&when[i]);
        put_long(when[i]);
        t_puts(" ");
        put_long(tm->tm_year + 1900L);
        t_puts("-");
        put2(tm->tm_mon + 1);
        t_puts("-");
        put2(tm->tm_mday);
        t_puts(" ");
        put2(tm->tm_hour);
        t_puts(":");
        put2(tm->tm_min);
        t_puts(":");
        put2(tm->tm_sec);
        t_puts(" wday ");
        put_long(tm->tm_wday);
        t_puts(" yday ");
        put_long(tm->tm_yday);
        t_puts(" dst ");
        put_long(tm->tm_isdst);
        t_puts("\r\n");
    }
    return 0;
}
