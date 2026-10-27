/* isxdigit - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl isxdigit(int c)
{
    return (unsigned)(c - '0') < 10 || (unsigned)((c | 0x20) - 'a') < 6;
}
