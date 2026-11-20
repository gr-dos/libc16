/* _dos_setftime - libc16 (docs/spec/dos.md) */

#include <dos.h>
#include "io16.h"

/* INT 21h 5701h */
static unsigned long set_ftime(int fd, unsigned date, unsigned time);
#pragma aux set_ftime = \
    "mov ax, 5701h" \
    "int 21h" \
    "sbb dx, dx" \
    parm [bx] [dx] [cx] value [dx ax] modify exact [ax dx];

unsigned __cdecl _dos_setftime(int handle, unsigned date, unsigned time)
{
    unsigned long r = set_ftime(handle, date, time);

    if (DOS_FAILED(r)) {
        __libc16_doserr(DOS_AX(r));
        return DOS_AX(r);
    }
    return 0;
}
