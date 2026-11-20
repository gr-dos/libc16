/* fdopen - libc16 (docs/spec/stdio.md) */

#include <errno.h>
#include <stdio.h>
#include "io16.h"
#include "stdio16.h"

FILE * __cdecl fdopen(int fd, const char *mode)
{
    FILE *fp;
    unsigned long r;
    unsigned char flag = _IOWRT, set = 0, keep = 0xFF, seen = 0, c;

    switch (*mode++) {
    case 'r':
        flag = _IOREAD;
        break;
    case 'a':
        set = FD_APPEND;
        /* fall through */
    case 'w':
        break;
    default:
        goto invalid;
    }
    /* Then "+", one of "b" and "t", both in either order, or neither. */
    while ((c = *mode++) != '\0') {
        if (c == '+' && !(seen & 1)) {
            flag = _IOREAD | _IOWRT;
            seen |= 1;
        } else if (c == 'b' && !(seen & 2)) {
            set |= FD_BINARY;
            seen |= 2;
        } else if (c == 't' && !(seen & 2)) {
            keep = ~FD_BINARY;
            seen |= 2;
        } else {
invalid:
            errno = EINVAL;
            return NULL;
        }
    }
    if ((unsigned)fd >= FD_MAX) {
        errno = EBADF;
        return NULL;
    }
    r = __libc16_dos_devinfo(fd);
    if (DOS_FAILED(r)) {
        _doserrno = DOS_AX(r);
        errno = EBADF;
        return NULL;
    }
    if ((fp = __libc16_stream()) == NULL)
        return NULL;
    __libc16_fdflags[fd] = (__libc16_fdflags[fd] & keep) | set;
    fp->_fd = fd;
    fp->_flag = flag;
    return fp;
}
