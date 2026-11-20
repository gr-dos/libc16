/* dup2 - libc16 (docs/spec/io.md) */

#include <errno.h>
#include <io.h>
#include "io16.h"

/* INT 21h 46h: makes fd2 refer to what fd refers to, closing fd2 first */
static unsigned long dos_dup2(int fd, int fd2);
#pragma aux dos_dup2 = \
    "mov ah, 46h" \
    "int 21h" \
    "sbb dx, dx" \
    parm [bx] [cx] value [dx ax] modify exact [ax dx];

int __cdecl dup2(int fd, int fd2)
{
    unsigned long r;

    /* fd2 must have a mode; fd must be open, checked before DOS can
     * close fd2 (and so that dup2(fd, fd) fails for a closed fd) */
    if ((unsigned)fd2 >= FD_MAX) {
        errno = EBADF;
        return -1;
    }
    r = __libc16_dos_devinfo(fd);
    if (DOS_FAILED(r)) {
        _doserrno = DOS_AX(r);
        errno = EBADF;
        return -1;
    }
    if (fd != fd2) {
        r = dos_dup2(fd, fd2);
        if (DOS_FAILED(r))
            return __libc16_doserr(DOS_AX(r));
        /* fd's mode; a byte kept for fd2 is discarded */
        __libc16_fdflags[fd2] = (unsigned)fd < FD_MAX ? __libc16_fdflags[fd] & ~FD_PENDING : 0;
    }
    return 0;
}
