/* _dos_close - libc16 (docs/spec/dos.md) */

#include <dos.h>
#include "io16.h"

unsigned __cdecl _dos_close(int handle)
{
    unsigned long r = __libc16_dos_close(handle);

    if (DOS_FAILED(r)) {
        __libc16_doserr(DOS_AX(r));
        return DOS_AX(r);
    }
    return 0;
}
