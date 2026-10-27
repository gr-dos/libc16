/* _dos_setfileattr - libc16 (docs/spec/dos.md) */

#include <dos.h>
#include "io16.h"

unsigned __cdecl _dos_setfileattr(const char *path, unsigned attr)
{
    unsigned long r = __libc16_dos_setattr(path, attr);

    if (DOS_FAILED(r)) {
        __libc16_doserr(DOS_AX(r));
        return DOS_AX(r);
    }
    return 0;
}
