/* strcmp - libc16 (docs/spec/string.md) */

#include <string.h>

int __cdecl strcmp(const char *s1, const char *s2)
{
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;

    for (; *a == *b; a++, b++)
        if (*a == '\0')
            return 0;
    return *a - *b;
}
