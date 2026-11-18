/* The child the process tests run (docs/spec/process.md, "Tests"): prints
 * its full path, its arguments, and on request its environment, a variable
 * or its first FCB, and exits with its last argument's value. */

#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "testio.h"

static void put_far(const char __far *s)
{
    char b[2];

    b[1] = 0;
    while (*s) {
        b[0] = *s++;
        t_puts(b);
    }
}

int main(int argc, char **argv)
{
    const char __far *env = MK_FP(*(unsigned __far *)MK_FP(_psp, 0x2c), 0);
    const char __far *p = env;
    int i;

    /* the full path DOS puts after the environment's two zero bytes and
     * the word 1 */
    if (*p == 0)
        p += 2;
    else {
        while (*p) {
            while (*p)
                p++;
            p++;
        }
        p++;
    }
    t_puts("name[");
    put_far(p + 2);
    t_puts("]\r\n");

    for (i = 1; i < argc; i++) {
        t_puts("arg[");
        t_puts(argv[i]);
        t_puts("]\r\n");
    }

    if (argc > 1 && strcmp(argv[1], "-e") == 0) {
        for (p = env; *p; ) {
            t_puts("env[");
            put_far(p);
            t_puts("]\r\n");
            while (*p)
                p++;
            p++;
        }
    }

    if (argc > 2 && strcmp(argv[1], "-g") == 0) {
        char *v = getenv(argv[2]);

        if (v == NULL)
            t_puts("val(null)\r\n");
        else {
            t_puts("val[");
            t_puts(v);
            t_puts("]\r\n");
        }
    }

    if (argc > 1 && strcmp(argv[1], "-f") == 0) {
        const char __far *fcb = MK_FP(_psp, 0x5c);
        char b[13];

        b[0] = '0' + fcb[0];
        for (i = 1; i < 12; i++)
            b[i] = fcb[i] ? fcb[i] : '.';
        b[12] = 0;
        t_puts("fcb[");
        t_puts(b);
        t_puts("]\r\n");
    }

    if (argc > 1 && argv[argc - 1][0] >= '0' && argv[argc - 1][0] <= '9')
        return atoi(argv[argc - 1]);
    return 0;
}
