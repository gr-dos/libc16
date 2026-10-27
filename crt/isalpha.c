/* isalpha - libc16 (docs/spec/ctype.md) */

#include <ctype.h>

int __cdecl isalpha(int c)
{
    return (unsigned)((c | 0x20) - 'a') < 26;
}
