/* CHECK(cond) for test programs: prints "not ok LINE" for each failed
 * check, and done() prints "ok" if every check passed and returns the
 * exit status. */

#ifndef CHECK_H
#define CHECK_H

#include "testio.h"

static int failures;

#define CHECK(cond) check((cond) != 0, __LINE__)

static void check(int ok, unsigned line)
{
    if (!ok) {
        t_puts("not ok ");
        t_putu(line);
        t_puts("\r\n");
        failures++;
    }
}

static int done(void)
{
    if (failures == 0)
        t_puts("ok\r\n");
    return failures != 0;
}

#endif
