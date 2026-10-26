/* strcat - libc16 (docs/spec/string.md) */

#include <string.h>

char * __cdecl strcat(char *dst, const char *src)
{
    char *d = dst;

    while (*d != '\0')
        d++;
    while ((*d++ = *src++) != '\0')
        ;
    return dst;
}
