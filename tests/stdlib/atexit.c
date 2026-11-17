/* atexit (docs/spec/startup.md): three functions, called in reverse order
 * when main returns; the exit status is still main's value. In the small
 * model they are called with DS = DGROUP. */

#include <stdlib.h>
#include "testio.h"

static unsigned get_ds(void);
#pragma aux get_ds = \
    "mov ax, ds" \
    value [ax];

static unsigned get_ss(void);
#pragma aux get_ss = \
    "mov ax, ss" \
    value [ax];

static void __cdecl a(void)
{
#if defined(__SMALL__)
    t_puts(get_ds() == get_ss() ? "a\r\n" : "a: DS is not DGROUP\r\n");
#else
    t_puts("a\r\n");
#endif
}

static void __cdecl b(void)
{
    t_puts("b\r\n");
}

static void __cdecl c(void)
{
    t_puts("c\r\n");
}

int main(void)
{
    if (atexit(a) != 0 || atexit(b) != 0 || atexit(c) != 0) {
        t_puts("not ok atexit\r\n");
        return 1;
    }
    t_puts("main\r\n");
    return 9;
}
