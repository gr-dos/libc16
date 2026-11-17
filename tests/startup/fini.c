/* Termination routines of the program's own (docs/spec/startup.md,
 * "Termination table"), from tests/startup/finitab.asm: an assembly one,
 * which with far data returns with DS = 0, then c_fini, a default-
 * convention C function. When main returns 9, both run once, in table
 * order, after main, and the exit status is still 9. */

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

int main(void)
{
    t_puts(fini_order == 0 && c_fini_calls == 0 ? "main\r\n" : "main: fini ran early\r\n");
    return 9;
}
