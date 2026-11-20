/* _dos_getftime - libc16 (docs/spec/dos.md) */

#include <dos.h>
#include "io16.h"

/* INT 21h 5700h: the date in AX, and the time stored at *time; or the
 * error code in AX, with *time unchanged */
static unsigned long get_ftime(int fd, unsigned *time);
#if defined(__COMPACT__) || defined(__LARGE__)
#pragma aux get_ftime = \
    "mov ax, 5700h" \
    "int 21h" \
    "jc short fail" \
    "mov es:[di], cx" \
    "mov ax, dx" \
    "fail:" \
    "sbb dx, dx" \
    parm [bx] [es di] value [dx ax] modify exact [ax cx dx];
#else
#pragma aux get_ftime = \
    "mov ax, 5700h" \
    "int 21h" \
    "jc short fail" \
    "mov [di], cx" \
    "mov ax, dx" \
    "fail:" \
    "sbb dx, dx" \
    parm [bx] [di] value [dx ax] modify exact [ax cx dx];
#endif

unsigned __cdecl _dos_getftime(int handle, unsigned *date, unsigned *time)
{
    unsigned t;
    unsigned long r = get_ftime(handle, &t);

    if (DOS_FAILED(r)) {
        __libc16_doserr(DOS_AX(r));
        return DOS_AX(r);
    }
    *date = DOS_AX(r);
    *time = t;
    return 0;
}
