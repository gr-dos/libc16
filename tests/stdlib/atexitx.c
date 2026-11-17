/* atexit (docs/spec/startup.md): three functions, called in reverse order
 * by exit from a nested call, which sets the exit status. */

#include <stdlib.h>
#include "testio.h"

static void __cdecl a(void)
{
    t_puts("a\r\n");
}

static void __cdecl b(void)
{
    t_puts("b\r\n");
}

static void __cdecl c(void)
{
    t_puts("c\r\n");
}

static void leave(int status)
{
    exit(status);
}

int main(void)
{
    if (atexit(a) != 0 || atexit(b) != 0 || atexit(c) != 0) {
        t_puts("not ok atexit\r\n");
        return 1;
    }
    t_puts("main\r\n");
    leave(0x306);
    t_puts("not ok exit returned\r\n");
    return 1;
}
