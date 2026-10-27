/* argv through far pointers (docs/spec/startup.md, "Command-tail
 * parser", "Memory models"): the argv array and its strings are in
 * DGROUP, whose segment is SS, and are modifiable. Upper-cases each
 * argument in place through a far pointer, replaces argv[0] with the last
 * argument, then prints a line for the segments and each argv[i] in
 * brackets. */

#include <dos.h>
#include "testio.h"

static unsigned get_ss(void);
#pragma aux get_ss = \
    "mov ax, ss" \
    value [ax];

static unsigned seg_of(const void __far *p)
{
    return FP_SEG(p);
}

int main(int argc, char **argv)
{
    unsigned ss = get_ss();
    int ok = seg_of(argv) == ss;
    char __far *p;
    int i;

    for (i = 0; i < argc; i++) {
        ok = ok && seg_of(argv[i]) == ss;
        for (p = argv[i]; *p != '\0'; p++)
            if (*p >= 'a' && *p <= 'z')
                *p -= 'a' - 'A';
    }
    argv[0] = argv[argc - 1];

    t_puts(ok ? "segments ok\r\n" : "segments wrong\r\n");
    for (i = 0; i < argc; i++) {
        t_puts("[");
        t_puts(argv[i]);
        t_puts("]\r\n");
    }
    return 0;
}
