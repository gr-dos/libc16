/* fflush - libc16 (docs/spec/stdio.md) */

#include <stdio.h>
#include "stdio16.h"

int __cdecl fflush(FILE *fp)
{
    /* Nothing is pending unless setvbuf or setbuf linked the engine. */
    return BUFCTL(fp, fp != NULL ? BUF_FLUSH : BUF_ALL);
}
