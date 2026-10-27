/* islower - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl islower(int c)
{
    return (unsigned)(c - 'a') < 26;
}
