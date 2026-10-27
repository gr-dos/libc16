/* isprint - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl isprint(int c)
{
    return (unsigned)(c - 0x20) < 0x5f;
}
