/* strlwr - libc16 (docs/spec/string.md) */

#include <string.h>

char * __cdecl strlwr(char *s)
{
    char *p;

    for (p = s; *p != '\0'; p++)
        if (*p >= 'A' && *p <= 'Z')
            *p += 'a' - 'A';
    return s;
}
