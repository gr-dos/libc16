/* isdigit - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl isdigit(int c)
{
    return (unsigned)(c - '0') < 10;
}
