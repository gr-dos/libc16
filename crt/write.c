/* write - libc16 (docs/spec/io.md) */

#include <errno.h>
#include <io.h>
#include "io16.h"

/* Writes n bytes untranslated: 0, or -1 with errno set. */
static int put(int fd, const void *buf, unsigned n)
{
    unsigned long r = __libc16_dos_write(fd, buf, n);

    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    if (DOS_AX(r) != n) {
        errno = ENOSPC;
        return -1;
    }
    return 0;
}

int __cdecl write(int fd, const void *buf, unsigned n)
{
    const char *p = buf;
    char tmp[128];
    unsigned i, len = 0;
    unsigned char flags = (unsigned)fd < FD_MAX ? __libc16_fdflags[fd] : FD_BINARY;

    if (n == 0)
        return 0;
    if (flags & FD_APPEND)
        __libc16_dos_seek(fd, 0, 2);
    if (flags & FD_BINARY)
        return put(fd, buf, n) == 0 ? (int)n : -1;
    for (i = 0; i < n; i++) {
        if (p[i] == '\n')
            tmp[len++] = '\r';
        tmp[len++] = p[i];
        if (len >= sizeof tmp - 1) {
            if (put(fd, tmp, len) != 0)
                return -1;
            len = 0;
        }
    }
    if (len != 0 && put(fd, tmp, len) != 0)
        return -1;
    return n;
}
