/* dup - libc16 (docs/spec/io.md) */

#include <errno.h>
#include <io.h>
#include "io16.h"

static unsigned long dos_dup(int fd);
#pragma aux dos_dup = \
    "mov ah, 45h" \
    "int 21h" \
    "sbb dx, dx" \
    parm [bx] value [dx ax] modify exact [ax dx];

int __cdecl dup(int fd)
{
    unsigned long r = dos_dup(fd);
    int fd2;

    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    fd2 = DOS_AX(r);
    /* A handle with no mode, as open refuses one */
    if (fd2 >= FD_MAX) {
        __libc16_dos_close(fd2);
        errno = EMFILE;
        return -1;
    }
    /* fd's mode; a byte kept for fd stays with it */
    __libc16_fdflags[fd2] = (unsigned)fd < FD_MAX ? __libc16_fdflags[fd] & ~FD_PENDING : 0;
    return fd2;
}
