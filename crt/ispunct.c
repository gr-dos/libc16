/* ispunct - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl ispunct(int c)
{
    return (unsigned)(c - 0x21) < 0x5e && (unsigned)(c - '0') >= 10 && (unsigned)((c | 0x20) - 'a') >= 26;
}
