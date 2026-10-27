/* isgraph - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl isgraph(int c)
{
    return (unsigned)(c - 0x21) < 0x5e;
}
