/* strlen - libc16 (docs/spec/string.md) */

#include <string.h>

size_t __cdecl strlen(const char *s)
{
    const char *p = s;

    while (*p != '\0')
        p++;
    return p - s;
}
