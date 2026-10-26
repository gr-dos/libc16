/* putchar - libc16 (docs/spec/stdio.md); <stdio.h> also has a macro */

#include <stdio.h>

#undef putchar

int __cdecl putchar(int c)
{
    return fputc(c, stdout);
}
