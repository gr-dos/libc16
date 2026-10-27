/* _dos_read - libc16 (docs/spec/dos.md) */

#include <dos.h>
#include "io16.h"

unsigned __cdecl _dos_read(int handle, void __far *buf, unsigned count, unsigned *nread)
{
    unsigned long r = __libc16_dos_read_far(handle, buf, count);

    if (DOS_FAILED(r)) {
        __libc16_doserr(DOS_AX(r));
        return DOS_AX(r);
    }
    *nread = DOS_AX(r);
    return 0;
}
