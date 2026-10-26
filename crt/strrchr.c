/* strrchr - libc16 (docs/spec/string.md) */

#include <string.h>

char * __cdecl strrchr(const char *s, int c)
{
    const char *found = NULL;

    for (;; s++) {
        if (*s == (char)c)
            found = s;
        if (*s == '\0')
            return (char *)found;
    }
}
