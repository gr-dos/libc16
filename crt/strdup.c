/* strdup - libc16 (docs/spec/string.md) */

#include <stdlib.h>
#include <string.h>

char * __cdecl strdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);

    if (p != NULL)
        memcpy(p, s, n);
    return p;
}
