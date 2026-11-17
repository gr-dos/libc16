/* strupr - libc16 (docs/spec/string.md) */

#include <string.h>

char * __cdecl strupr(char *s)
{
    char *p;

    for (p = s; *p != '\0'; p++)
        if (*p >= 'a' && *p <= 'z')
            *p -= 'a' - 'A';
    return s;
}
