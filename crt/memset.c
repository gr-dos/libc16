/* memset - libc16 (docs/spec/string.md) */

#include <string.h>

void * __cdecl memset(void *s, int c, size_t n)
{
    unsigned char *p = s;

    while (n-- != 0)
        *p++ = (unsigned char)c;
    return s;
}
