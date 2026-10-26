/* Prints __libc16_stack_top, the top of the stack (docs/spec/startup.md),
 * which tests/startup/check-memory compares with the end of DGROUP in
 * the map, and whether main's local variable lies just below it. */

#include "testio.h"

extern char __cdecl __libc16_stack_top[];

int main(void)
{
    char local;
    unsigned below = (unsigned)__libc16_stack_top - (unsigned)&local;

    t_puts("top ");
    t_putu((unsigned)__libc16_stack_top);
    t_puts("\r\nlocal ");
    t_puts(below > 0 && below <= 32 ? "below" : "elsewhere");
    t_puts("\r\n");
    return 0;
}
