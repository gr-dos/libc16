/* memcmp - libc16 (docs/spec/string.md) */

#include <string.h>

int __cdecl memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *a = s1, *b = s2;

    for (; n != 0; n--, a++, b++)
        if (*a != *b)
            return *a - *b;
    return 0;
}
