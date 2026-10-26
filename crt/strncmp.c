/* strncmp - libc16 (docs/spec/string.md) */

#include <string.h>

int __cdecl strncmp(const char *s1, const char *s2, size_t n)
{
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;

    for (; n != 0; n--, a++, b++) {
        if (*a != *b)
            return *a - *b;
        if (*a == '\0')
            break;
    }
    return 0;
}
