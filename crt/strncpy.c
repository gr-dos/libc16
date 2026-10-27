/* strncpy - libc16 (docs/spec/string.md) */

#include <string.h>

char * __cdecl strncpy(char *dst, const char *src, size_t n)
{
    char *d = dst;

    for (; n != 0 && *src != '\0'; n--)
        *d++ = *src++;
    for (; n != 0; n--)
        *d++ = '\0';
    return dst;
}
