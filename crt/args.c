/* libc16 command-tail parser: builds argc and argv for main.
 *
 * Written from docs/spec/startup.md ("Command-tail parser"), the PSP format
 * table in Ralf Brown's Interrupt List (under INT 21h function 26h:
 * command-tail length at offset 80h, text from 81h, ended by a carriage
 * return), Microsoft's documentation of how Microsoft C parses command-line
 * arguments, and C99 5.1.2.2.1. Runs from the init table (crt/argv.asm). */

#include "startup.h"

#define TAIL_MAX 127            /* bytes in a command tail */
#define ARGS_MAX 64             /* most arguments TAIL_MAX bytes can split into */

/* text[0] is argv[0]'s empty string; the tail is copied after it and split
 * in place. Splitting never writes ahead of where it reads (each output
 * byte, and each terminator but the last, consumes at least one input
 * byte), so one byte past the tail is enough for the last terminator. */
static char text[1 + TAIL_MAX + 1];

/* argv[0], the arguments and the null pointer after them. */
static char *vec[1 + ARGS_MAX + 1];

void __libc16_args(void)
{
    const char __far *tail = ((__segment)_psp) :> ((const char __near *)0x80);
    unsigned n;
    unsigned slashes;
    char *src;
    char *dst;
    char *lim;
    char c;
    int argc;
    int quoted;

    /* Copy the tail, which ends at its length or at a carriage return. */
    n = (unsigned char)*tail;
    if (n > TAIL_MAX)
        n = TAIL_MAX;
    lim = text + 1;
    while (n-- != 0 && (c = *++tail) != '\r')
        *lim++ = c;

    /* BSS is zero, so text[0] already ends argv[0] and every vec entry
     * past the last argument is already a null pointer. */
    vec[0] = text;
    argc = 1;
    src = text + 1;
    for (;;) {
        while (src < lim && (*src == ' ' || *src == '\t'))
            src++;
        if (src == lim)
            break;
        vec[argc++] = dst = src;
        quoted = 0;
        for (;;) {
            slashes = 0;
            while (src < lim && *src == '\\') {
                src++;
                slashes++;
            }
            if (src < lim && *src == '"') {
                /* 2n backslashes and a quote: n backslashes, and the
                 * quote toggles quoting; 2n + 1: n backslashes and a
                 * literal quote. */
                src++;
                for (; slashes >= 2; slashes -= 2)
                    *dst++ = '\\';
                if (slashes != 0)
                    *dst++ = '"';
                else
                    quoted = !quoted;
                continue;
            }
            /* Backslashes not followed by a quote are literal. */
            for (; slashes != 0; slashes--)
                *dst++ = '\\';
            if (src == lim)
                break;
            c = *src++;
            if (!quoted && (c == ' ' || c == '\t'))
                break;          /* the separator is consumed, so dst < src */
            *dst++ = c;
        }
        *dst = '\0';
    }
    __libc16_argc = argc;
    __libc16_argv = vec;
}
