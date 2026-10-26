/* fflush - libc16 (docs/spec/stdio.md); output is unbuffered */

#include <stdio.h>

int __cdecl fflush(FILE *fp)
{
    (void)fp;
    return 0;
}
