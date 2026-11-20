/* memchr - libc16 (docs/spec/string.md) */

#include <string.h>

void * __cdecl memchr(const void *s, int c, size_t n)
{
    const unsigned char *p = s;
    unsigned char target = (unsigned char)c;

    for (; n != 0; n--, p++)
        if (*p == target)
            return (void *)p;
    return NULL;
}
