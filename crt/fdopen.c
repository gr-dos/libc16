/* fdopen - libc16 (docs/spec/stdio.md) */

#include <errno.h>
#include <stdio.h>
#include "io16.h"
#include "stdio16.h"

FILE * __cdecl fdopen(int fd, const char *mode)
{
    FILE *fp;
    unsigned char flag = _IOWRT, set = 0, keep = 0xFF;

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
    if (*mode == 'b') {
        set |= FD_BINARY;
        mode++;
    } else if (*mode == 't') {
        keep = ~FD_BINARY;
        mode++;
    }
    if (*mode != '\0') {
invalid:
        errno = EINVAL;
        return NULL;
    }
    if ((unsigned)fd >= FD_MAX || DOS_FAILED(__libc16_dos_devinfo(fd))) {
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
