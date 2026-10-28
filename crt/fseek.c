/* fseek - libc16 (docs/spec/stdio.md) */

#include <errno.h>
#include <io.h>
#include <stdio.h>
#include "io16.h"

#define SEEK_FAILED(r) ((unsigned)((r) >> 16) == 0xffff)

int __cdecl fseek(FILE *fp, long offset, int origin)
{
    unsigned long r, here;
    long target;

    switch (origin) {
    case SEEK_SET:
        target = offset;
        break;
    case SEEK_CUR:
        target = ftell(fp);
        if (target < 0)
            return -1;
        target += offset;
        break;
    case SEEK_END:
        /* Find the end, then put the handle back as it was. */
        here = __libc16_dos_seek(fp->_fd, 0L, SEEK_CUR);
        if (SEEK_FAILED(here))
            return __libc16_doserr(DOS_AX(here));
        r = __libc16_dos_seek(fp->_fd, 0L, SEEK_END);
        __libc16_dos_seek(fp->_fd, (long)here, SEEK_SET);
        if (SEEK_FAILED(r))
            return __libc16_doserr(DOS_AX(r));
        target = (long)r + offset;
        break;
    default:
        errno = EINVAL;
        return -1;
    }
    if (target < 0) {
        errno = EINVAL;
        return -1;
    }
    if (lseek(fp->_fd, target, SEEK_SET) < 0)
        return -1;
    fp->_cnt = 0;
    fp->_ptr = fp->_base;
    fp->_flag &= ~(_IOEOF | _IOUNGET);
    return 0;
}
