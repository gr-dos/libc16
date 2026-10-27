/* _dos_write - libc16 (docs/spec/dos.md) */

#include <dos.h>
#include "io16.h"

unsigned __cdecl _dos_write(int handle, const void __far *buf, unsigned count, unsigned *nwritten)
{
    unsigned long r = __libc16_dos_write_far(handle, buf, count);

    if (DOS_FAILED(r)) {
        __libc16_doserr(DOS_AX(r));
        return DOS_AX(r);
    }
    *nwritten = DOS_AX(r);
    return 0;
}
