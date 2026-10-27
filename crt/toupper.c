/* toupper - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl toupper(int c)
{
    return (unsigned)(c - 'a') < 26 ? c - ('a' - 'A') : c;
}
