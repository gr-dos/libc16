/* lseek - libc16 (docs/spec/io.md) */

#include <errno.h>
#include <io.h>
#include <stdio.h>
#include "io16.h"

#define SEEK_FAILED(r) ((unsigned)((r) >> 16) == 0xffff)

long __cdecl lseek(int fd, long offset, int origin)
{
    unsigned long r, here;

    if ((unsigned)origin > SEEK_END) {
        errno = EINVAL;
        return -1L;
    }
    /* A byte kept by a text-mode read is past the DOS file pointer. */
    if ((unsigned)fd < FD_MAX && (__libc16_fdflags[fd] & FD_PENDING)) {
        __libc16_fdflags[fd] &= ~FD_PENDING;
        if (origin == SEEK_CUR)
            offset--;
    }
    /* DOS does not reliably refuse a position before the start, so a
     * negative offset is made absolute first. */
    if (offset < 0 && origin != SEEK_SET) {
        here = __libc16_dos_seek(fd, 0L, SEEK_CUR);
        r = here;
        if (!SEEK_FAILED(r) && origin == SEEK_END)
            r = __libc16_dos_seek(fd, 0L, SEEK_END);
        if (SEEK_FAILED(r))
            return __libc16_doserr(DOS_AX(r));
        offset += (long)r;
        origin = SEEK_SET;
        if (offset < 0)
            __libc16_dos_seek(fd, (long)here, SEEK_SET);
    }
    if (offset < 0) {
        errno = EINVAL;
        return -1L;
    }
    r = __libc16_dos_seek(fd, offset, origin);
    if (SEEK_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    return (long)r;
}
