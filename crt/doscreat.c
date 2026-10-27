/* _dos_creat - libc16 (docs/spec/dos.md) */

#include <dos.h>
#include "io16.h"

unsigned __cdecl _dos_creat(const char *path, unsigned attr, int *handle)
{
    unsigned long r = __libc16_dos_creat(path, attr);

    if (DOS_FAILED(r)) {
        __libc16_doserr(DOS_AX(r));
        return DOS_AX(r);
    }
    *handle = DOS_AX(r);
    return 0;
}
