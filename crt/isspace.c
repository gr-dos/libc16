/* isspace - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl isspace(int c)
{
    return c == ' ' || (unsigned)(c - '\t') < 5;
}
