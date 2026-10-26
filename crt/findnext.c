/* _dos_findnext - libc16 (docs/spec/dos.md) */

#include "dos16.h"

unsigned __cdecl _dos_findnext(struct find_t *f)
{
    return __libc16_find(0x4f, (const char *)0, 0, f);
}
