/* abort does not write buffered output (docs/spec/stdio.md,
 * "Termination"): what is pending is lost; what was flushed is not. */

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    FILE *fp = fopen("BUFABRT.TXT", "w");

    if (fp == NULL || setvbuf(fp, NULL, _IOFBF, 0) != 0 || setvbuf(stdout, NULL, _IOFBF, 0) != 0)
        return 1;
    fputs("flushed\n", stdout);
    fputs("flushed\n", fp);
    if (fflush(stdout) != 0 || fflush(fp) != 0)
        return 1;
    fputs("lost\n", stdout);
    fputs("lost\n", fp);
    abort();
    return 0;
}
