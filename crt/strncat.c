/* strncat - libc16 (docs/spec/string.md) */

#include <string.h>

char * __cdecl strncat(char *dst, const char *src, size_t n)
{
    char *d = dst;

    while (*d != '\0')
        d++;
    for (; n != 0 && *src != '\0'; n--)
        *d++ = *src++;
    *d = '\0';
    return dst;
}
