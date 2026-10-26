/* chsize - libc16 (docs/spec/io.md) */

#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include "io16.h"

int __cdecl chsize(int fd, long size)
{
    char zeros[512];
    long here, end;
    unsigned long r;
    int rc = 0, err;

    if (size < 0) {
        errno = EINVAL;
        return -1;
    }
    if ((here = lseek(fd, 0L, SEEK_CUR)) == -1L)
        return -1;
    if ((end = lseek(fd, 0L, SEEK_END)) == -1L) {
        rc = -1;
    } else if (size < end) {
        if (lseek(fd, size, SEEK_SET) == -1L) {
            rc = -1;
        } else {
            r = __libc16_dos_write(fd, zeros, 0);
            if (DOS_FAILED(r))
                rc = __libc16_doserr(DOS_AX(r));
        }
    } else {
        memset(zeros, 0, sizeof zeros);
        while (end < size) {
            unsigned n = size - end > (long)sizeof zeros ? sizeof zeros : (unsigned)(size - end);

            r = __libc16_dos_write(fd, zeros, n);
            if (DOS_FAILED(r)) {
                rc = __libc16_doserr(DOS_AX(r));
                break;
            }
            if (DOS_AX(r) != n) {
                errno = ENOSPC;
                rc = -1;
                break;
            }
            end += n;
        }
    }
    /* A failure to restore the position is reported, but does not
     * replace an earlier error. */
    err = errno;
    if (lseek(fd, here, SEEK_SET) == -1L) {
        if (rc == 0)
            rc = -1;
        else
            errno = err;
    }
    return rc;
}
