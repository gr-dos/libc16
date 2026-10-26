/* strcpy - libc16 (docs/spec/string.md) */

#include <string.h>

char * __cdecl strcpy(char *dst, const char *src)
{
    char *d = dst;

    while ((*d++ = *src++) != '\0')
        ;
    return dst;
}
