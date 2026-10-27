/* tolower - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl tolower(int c)
{
    return (unsigned)(c - 'A') < 26 ? c + ('a' - 'A') : c;
}
