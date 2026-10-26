/* open - libc16 (docs/spec/io.md) */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdarg.h>
#include <sys/stat.h>
#include "io16.h"

int __cdecl open(const char *path, int oflag, ...)
{
    unsigned char acc = oflag & 3;
    unsigned long r;
    int fd, created = 0;

    if (acc == 3 || (oflag & (O_TEXT | O_BINARY)) == (O_TEXT | O_BINARY)) {
        errno = EINVAL;
        return -1;
    }
    r = __libc16_dos_open(path, acc);
    if (oflag & O_CREAT) {
        if (!DOS_FAILED(r) && (oflag & O_EXCL)) {
            __libc16_dos_close(DOS_AX(r));
            errno = EEXIST;
            return -1;
        }
        if (DOS_FAILED(r) && DOS_AX(r) == 2) {
            va_list ap;
            int pmode;

            va_start(ap, oflag);
            pmode = va_arg(ap, int);
            va_end(ap);
            r = __libc16_dos_creat(path, (pmode & S_IWRITE) ? 0 : 1);
            created = 1;
        }
    }
    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    fd = DOS_AX(r);
    if (fd >= FD_MAX) {
        __libc16_dos_close(fd);
        errno = EMFILE;
        return -1;
    }
    if ((oflag & O_TRUNC) && !created && acc != O_RDONLY) {
        r = __libc16_dos_write(fd, "", 0);
        if (DOS_FAILED(r)) {
            __libc16_dos_close(fd);
            return __libc16_doserr(DOS_AX(r));
        }
    }
    __libc16_fdflags[fd] = ((oflag & O_BINARY) ? FD_BINARY : 0)
                         | ((oflag & O_APPEND) ? FD_APPEND : 0);
    return fd;
}
