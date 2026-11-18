/* The child the process tests run (docs/spec/process.md, "Tests"): prints
 * its command tail as DOS gave it if TAIL is set in its environment, its
 * full path, its arguments, and on request its environment, a variable or
 * its first FCB, and exits with its last argument's value. Run as a
 * command processor, its first argument is /C; its options follow it. */

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
    int i, o;

    /* the command tail, as DOS gave it: its length byte, then the bytes */
    if (getenv("TAIL") != NULL) {
        const char __far *t = MK_FP(_psp, 0x80);
        char b[2];

        b[1] = 0;
        t_puts("tail[");
        for (i = 1; i <= t[0]; i++) {
            b[0] = t[i];
            t_puts(b);
        }
        t_puts("]\r\n");
    }

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

    o = argc > 1 && strcmp(argv[1], "/C") == 0 ? 2 : 1;

    if (argc > o && strcmp(argv[o], "-e") == 0) {
        for (p = env; *p; ) {
            t_puts("env[");
            put_far(p);
            t_puts("]\r\n");
            while (*p)
                p++;
            p++;
        }
    }

    if (argc > o + 1 && strcmp(argv[o], "-g") == 0) {
        char *v = getenv(argv[o + 1]);

        if (v == NULL)
            t_puts("val(null)\r\n");
        else {
            t_puts("val[");
            t_puts(v);
            t_puts("]\r\n");
        }
    }

    if (argc > o && strcmp(argv[o], "-f") == 0) {
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
