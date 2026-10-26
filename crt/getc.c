/* getc - libc16 (docs/spec/stdio.md); <stdio.h> also has a macro */

#include <stdio.h>

#undef getc

int __cdecl getc(FILE *fp)
{
    return fgetc(fp);
}
