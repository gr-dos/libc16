/* strnicmp - libc16 (docs/spec/string.md) */

#include <string.h>

int __cdecl strnicmp(const char *s1, const char *s2, size_t n)
{
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;
    int ca, cb;

    for (; n != 0; n--) {
        ca = *a++;
        cb = *b++;
        if (ca >= 'A' && ca <= 'Z')
            ca += 'a' - 'A';
        if (cb >= 'A' && cb <= 'Z')
            cb += 'a' - 'A';
        if (ca != cb)
            return ca - cb;
        if (ca == '\0')
            break;
    }
    return 0;
}
