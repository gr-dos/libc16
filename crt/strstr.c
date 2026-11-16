/* strstr - libc16 (docs/spec/string.md) */

#include <string.h>

char * __cdecl strstr(const char *s, const char *find)
{
    const char *a, *b;

    for (;; s++) {
        /* compare find with s, up to find's terminator */
        for (a = s, b = find; *b != '\0' && *a == *b; a++, b++)
            ;
        if (*b == '\0')
            return (char *)s;
        if (*s == '\0')
            return NULL;
    }
}
