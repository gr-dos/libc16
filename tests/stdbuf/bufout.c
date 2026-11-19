/* Buffered standard output reaches DOS at termination (docs/spec/stdio.md,
 * "Buffering", "Termination"): returning from main writes it, and output
 * from an atexit function follows it, whichever of the two termination
 * routines runs first. tests/stdbuf/bufout.expect checks the order. */

#include <stdio.h>
#include <stdlib.h>
#include "testio.h"

static void __cdecl late(void)
{
    /* the stream may be unbuffered by now; either way this comes last */
    fputs("from atexit\n", stdout);
    /* too late for a buffer: termination has begun, or will flush it */
    setvbuf(stdout, NULL, _IOFBF, 0);
    fputs("after setvbuf\n", stdout);
}

int main(void)
{
    unsigned i;

    if (atexit(late) != 0 || setvbuf(stdout, NULL, _IOFBF, 0) != 0)
        return 1;
    /* t_puts writes straight to handle 1, ahead of the pending output */
    fputs("pending\n", stdout);
    t_puts("direct\r\n");
    for (i = 0; i < 100; i++)
        fputs("line of buffered output\n", stdout);
    fputs("last, no newline", stdout);
    return 7;
}
