/* atexit (docs/spec/startup.md) holds 32 functions: report, then count 31
 * times. A 33rd registration fails. At termination count is called 31
 * times and report last. */

#include <stdlib.h>
#include "testio.h"

static unsigned calls;

static void __cdecl count(void)
{
    calls++;
}

static void __cdecl report(void)
{
    t_puts("count called ");
    t_putu(calls);
    t_puts(" times\r\n");
}

int main(void)
{
    unsigned ok = 0, i;

    if (atexit(report) == 0)
        ok++;
    for (i = 0; i < 31; i++)
        if (atexit(count) == 0)
            ok++;
    t_puts("registered ");
    t_putu(ok);
    t_puts("\r\n");
    t_puts(atexit(count) != 0 ? "33rd fails\r\n" : "not ok 33rd registered\r\n");
    t_puts(atexit(report) != 0 ? "34th fails\r\n" : "not ok 34th registered\r\n");
    return 0;
}
