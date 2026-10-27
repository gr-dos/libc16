/* isupper - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl isupper(int c)
{
    return (unsigned)(c - 'A') < 26;
}
