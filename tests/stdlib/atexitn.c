/* atexit (docs/spec/startup.md) during termination: f3 registers f4,
 * which is called next; f2 calls exit(4), after which only f1 is left.
 * Each function runs once, and the exit status is 4. */

#include <stdlib.h>
#include "testio.h"

static void __cdecl f1(void)
{
    t_puts("f1\r\n");
}

static void __cdecl f2(void)
{
    t_puts("f2\r\n");
    exit(4);
}

static void __cdecl f4(void)
{
    t_puts("f4\r\n");
}

static void __cdecl f3(void)
{
    t_puts(atexit(f4) == 0 ? "f3 registered f4\r\n" : "not ok f3 could not register f4\r\n");
}

static void __cdecl f5(void)
{
    t_puts("f5\r\n");
}

int main(void)
{
    if (atexit(f1) != 0 || atexit(f2) != 0 || atexit(f3) != 0 || atexit(f5) != 0) {
        t_puts("not ok atexit\r\n");
        return 1;
    }
    t_puts("main\r\n");
    exit(8);
    return 1;
}
