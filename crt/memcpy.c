/* memcpy - libc16 (docs/spec/string.md) */

#include <string.h>

void * __cdecl memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;

    while (n-- != 0)
        *d++ = *s++;
    return dst;
}
