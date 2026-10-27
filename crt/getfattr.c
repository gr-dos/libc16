/* _dos_getfileattr - libc16 (docs/spec/dos.md) */

#include <dos.h>
#include "io16.h"

unsigned __cdecl _dos_getfileattr(const char *path, unsigned *attr)
{
    unsigned long r = __libc16_dos_getattr(path);

    if (DOS_FAILED(r)) {
        __libc16_doserr(DOS_AX(r));
        return DOS_AX(r);
    }
    *attr = DOS_AX(r);
    return 0;
}
