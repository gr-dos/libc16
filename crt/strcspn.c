/* strcspn - libc16 (docs/spec/string.md) */

#include <string.h>

size_t __cdecl strcspn(const char *s, const char *reject)
{
    const char *p, *r;

    for (p = s; *p != '\0'; p++) {
        for (r = reject; *r != '\0'; r++) {
            if (*r == *p)
                return p - s;
        }
    }
    return p - s;
}
