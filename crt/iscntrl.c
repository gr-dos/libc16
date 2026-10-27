/* iscntrl - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl iscntrl(int c)
{
    return (unsigned)c < 0x20 || c == 0x7f;
}
