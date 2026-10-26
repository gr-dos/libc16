/* close - libc16 (docs/spec/io.md) */

#include <io.h>
#include "io16.h"

int __cdecl close(int fd)
{
    unsigned long r = __libc16_dos_close(fd);

    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    if ((unsigned)fd < FD_MAX)
        __libc16_fdflags[fd] = 0;
    return 0;
}
