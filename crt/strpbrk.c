/* strpbrk - libc16 (docs/spec/string.md) */

#include <string.h>

char * __cdecl strpbrk(const char *s, const char *accept)
{
    const char *a;

    for (; *s != '\0'; s++) {
        for (a = accept; *a != '\0'; a++) {
            if (*a == *s)
                return (char *)s;
        }
    }
    return NULL;
}
