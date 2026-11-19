/* fseek - libc16 (docs/spec/stdio.md) */

#include <errno.h>
#include <stdio.h>
#include "io16.h"
#include "stdio16.h"

#define SEEK_FAILED(r) ((unsigned)((r) >> 16) == 0xffff)

int __cdecl fseek(FILE *fp, long offset, int origin)
{
    int fd = fp->_fd;
    unsigned long here, r;

    if (origin != SEEK_SET && origin != SEEK_CUR && origin != SEEK_END) {
        errno = EINVAL;
        return -1;
    }
    /* Pending output goes where it was written, before the handle moves. */
    if ((fp->_flag & _IOWBUF) && __libc16_bufctl(fp, BUF_FLUSH) != 0)
        return -1;
    /* The target, before anything else changes. A failed DOS seek does not
     * move the handle; only SEEK_END's moves it before the last. */
    if (origin == SEEK_CUR) {
        if ((r = ftell(fp)) == (unsigned long)-1L)
            return -1;
        offset += (long)r;
    } else if (origin == SEEK_END) {
        here = __libc16_dos_seek(fd, 0L, SEEK_CUR);
        if (SEEK_FAILED(here))
            return __libc16_doserr(DOS_AX(here));
        r = __libc16_dos_seek(fd, 0L, SEEK_END);
        if (SEEK_FAILED(r))
            return __libc16_doserr(DOS_AX(r));
        offset += (long)r;
        if (offset < 0) {
            r = __libc16_dos_seek(fd, (long)here, SEEK_SET);
            if (SEEK_FAILED(r))
                return __libc16_seekback(fp, here, DOS_AX(r));
        }
    }
    if (offset < 0) {
        errno = EINVAL;
        return -1;
    }
    r = __libc16_dos_seek(fd, offset, SEEK_SET);
    if (SEEK_FAILED(r)) {
        if (origin == SEEK_END)
            return __libc16_seekback(fp, here, DOS_AX(r));
        return __libc16_doserr(DOS_AX(r));
    }
    /* The buffer, a pushed-back byte and a byte kept by read are before
     * or after the new position, not at it. */
    fp->_cnt = 0;
    fp->_flag &= ~(_IOUNGET | _IOEOF);
    if ((unsigned)fd < FD_MAX)
        __libc16_fdflags[fd] &= ~FD_PENDING;
    return 0;
}
