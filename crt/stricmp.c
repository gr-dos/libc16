/* stricmp - libc16 (docs/spec/string.md) */

#include <string.h>

int __cdecl stricmp(const char *s1, const char *s2)
{
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;
    int ca, cb;

    do {
        ca = *a++;
        cb = *b++;
        if (ca >= 'A' && ca <= 'Z')
            ca += 'a' - 'A';
        if (cb >= 'A' && cb <= 'Z')
            cb += 'a' - 'A';
    } while (ca == cb && ca != '\0');
    return ca - cb;
}
