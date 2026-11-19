/* exit from a nested function writes buffered output, to standard output
 * and to a file (docs/spec/stdio.md, "Termination"); bufchk checks the
 * file. */

#include <stdio.h>
#include <stdlib.h>

static char lbuf[64];

static void finish(FILE *fp)
{
    fputs("file pending", fp);
    fputs("stdout pending\n", stdout);
    exit(5);
}

int main(void)
{
    FILE *fp = fopen("BUFEXIT.TXT", "w");

    if (fp == NULL || setvbuf(fp, NULL, _IOFBF, 4096) != 0
            || setvbuf(stdout, lbuf, _IOLBF, sizeof lbuf) != 0)
        return 1;
    fputs("line\n", stdout);
    fputs("partial ", stdout);
    finish(fp);
    return 0;
}
