/* setbuf with the caller's buffer needs no heap (docs/spec/stdio.md,
 * "Buffering", "Linking"): tests/stdbuf/members checks that this program
 * links neither malloc nor free, and setbuf.expect its output, which
 * termination writes. */

#include <stdio.h>

static char buf[BUFSIZ];

int main(void)
{
    unsigned i;

    setbuf(stdout, buf);
    for (i = 0; i < 30; i++)
        fputs("buffered in the program's own BUFSIZ bytes\n", stdout);
    return 0;
}
