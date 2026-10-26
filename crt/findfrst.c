/* _dos_findfirst - libc16 (docs/spec/dos.md) */

#include "dos16.h"

unsigned __cdecl _dos_findfirst(const char *path, unsigned attr, struct find_t *f)
{
    return __libc16_find(0x4e, path, attr, f);
}
