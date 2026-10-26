/* access - libc16 (docs/spec/io.md) */

#include <dos.h>
#include <errno.h>
#include <io.h>
#include "io16.h"

int __cdecl access(const char *path, int mode)
{
    unsigned long r;

    if ((mode & ~6) != 0) {
        errno = EINVAL;
        return -1;
    }
    r = __libc16_dos_getattr(path);
    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    if ((mode & 2) && (DOS_AX(r) & _A_RDONLY)) {
        errno = EACCES;
        return -1;
    }
    return 0;
}
