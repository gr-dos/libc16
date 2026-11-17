/* As tests/startup/fini, but the program ends with exit(6) from a nested
 * call: exit runs the termination table, and the exit status is 6. */

#include <stdlib.h>
#include "testio.h"

void c_fini(void);

int fini_order;                 /* 1 after the assembly routine has run once */
static int c_fini_calls;

void c_fini(void)
{
    c_fini_calls++;
    t_puts("fini ");
    t_puts(fini_order == 1 && c_fini_calls == 1 ? "ok" : "wrong");
    t_puts("\r\n");
}

static void leave(int status)
{
    exit(status);
}

int main(void)
{
    t_puts(fini_order == 0 && c_fini_calls == 0 ? "main\r\n" : "main: fini ran early\r\n");
    leave(0x7706);
    t_puts("not ok exit returned\r\n");
    return 1;
}
