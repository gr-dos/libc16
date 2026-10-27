/* _fmemcpy - libc16 (docs/spec/string.md) */

#include <string.h>

void __far * __cdecl _fmemcpy(void __far *dst, const void __far *src, size_t n)
{
    unsigned char __far *d = dst;
    const unsigned char __far *s = src;

    while (n-- != 0)
        *d++ = *s++;
    return dst;
}
