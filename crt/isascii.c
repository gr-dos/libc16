/* isascii - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl isascii(int c)
{
    return (unsigned)c < 0x80;
}
