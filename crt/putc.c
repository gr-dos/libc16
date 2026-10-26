/* putc - libc16 (docs/spec/stdio.md); <stdio.h> also has a macro */

#include <stdio.h>

#undef putc

int __cdecl putc(int c, FILE *fp)
{
    return fputc(c, fp);
}
