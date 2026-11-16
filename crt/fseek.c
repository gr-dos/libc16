/* fseek - libc16 (docs/spec/stdio.md) */

#include <errno.h>
#include <stdio.h>
#include "io16.h"

#define SEEK_FAILED(r) ((unsigned)((r) >> 16) == 0xffff)

int __cdecl fseek(FILE *fp, long offset, int origin)
{
    int fd = fp->_fd;
    unsigned long here, r;

    /* The target, before anything changes */
    if (origin == SEEK_CUR) {
        if ((r = ftell(fp)) == (unsigned long)-1L)
            return -1;
        offset += (long)r;
    } else if (origin == SEEK_END) {
        here = __libc16_dos_seek(fd, 0L, SEEK_CUR);
        r = here;
        if (!SEEK_FAILED(r)) {
            r = __libc16_dos_seek(fd, 0L, SEEK_END);
            __libc16_dos_seek(fd, (long)here, SEEK_SET);
        }
        if (SEEK_FAILED(r))
            goto discard;
        offset += (long)r;
    } else if (origin != SEEK_SET) {
        offset = -1L;
    }
    if (offset < 0) {
        errno = EINVAL;
        return -1;
    }
    r = __libc16_dos_seek(fd, offset, SEEK_SET);
    if (!SEEK_FAILED(r))
        fp->_flag &= ~_IOEOF;
discard:
    /* The buffer, a pushed-back byte and a byte kept by read are before
     * or after the new position, not at it. */
    fp->_cnt = 0;
    fp->_flag &= ~_IOUNGET;
    if ((unsigned)fd < FD_MAX)
        __libc16_fdflags[fd] &= ~FD_PENDING;
    if (SEEK_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    return 0;
}
