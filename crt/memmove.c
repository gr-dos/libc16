/* memmove - libc16 (docs/spec/string.md) */

#include <string.h>

void * __cdecl memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;

    /* Overlapping objects are one object, so their pointers share a
     * segment and comparing them compares offsets. */
    if (d <= s) {
        while (n-- != 0)
            *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n-- != 0)
            *--d = *--s;
    }
    return dst;
}
