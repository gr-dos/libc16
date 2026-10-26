/* strchr - libc16 (docs/spec/string.md) */

#include <string.h>

char * __cdecl strchr(const char *s, int c)
{
    for (;; s++) {
        if (*s == (char)c)
            return (char *)s;
        if (*s == '\0')
            return NULL;
    }
}
