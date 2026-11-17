/* isatty - libc16 (docs/spec/io.md) */

#include <errno.h>
#include <io.h>
#include "io16.h"

int __cdecl isatty(int fd)
{
    unsigned long r = __libc16_dos_devinfo(fd);

    if (DOS_FAILED(r)) {
        errno = EBADF;
        return 0;
    }
    return ((unsigned)r >> 7) & 1;
}
