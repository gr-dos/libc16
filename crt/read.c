/* read - libc16 (docs/spec/io.md) */

#include <io.h>
#include "io16.h"

#define CR   '\r'
#define LF   '\n'
#define CTLZ 0x1a

int __cdecl read(int fd, void *buf, unsigned n)
{
    unsigned char *p = buf, *out, *end;
    unsigned char *flags;
    unsigned long r;
    unsigned raw = 0;

    /* DOS checks a handle outside the table */
    if ((unsigned)fd >= FD_MAX || (__libc16_fdflags[fd] & FD_BINARY)) {
        r = __libc16_dos_read(fd, buf, n);
        if (DOS_FAILED(r))
            return __libc16_doserr(DOS_AX(r));
        return DOS_AX(r);
    }
    if (n == 0)
        return 0;
    flags = &__libc16_fdflags[fd];
    /* A byte that followed a CR at the end of the last read comes first;
     * it is already past the DOS file pointer, just before what follows. */
    if (*flags & FD_PENDING) {
        *p = __libc16_fdpend[fd];
        *flags &= ~FD_PENDING;
        raw = 1;
    }
    if (raw < n) {
        r = __libc16_dos_read(fd, p + raw, n - raw);
        if (DOS_FAILED(r))
            return __libc16_doserr(DOS_AX(r));
        raw += DOS_AX(r);
    }
    out = p;
    end = p + raw;
    for (; p < end; p++) {
        if (*p == CTLZ) {
            /* leave the file pointer at the ^Z */
            __libc16_dos_seek(fd, -(long)(end - p), 1);
            break;
        }
        if (*p == CR) {
            if (p + 1 < end) {
                if (p[1] == LF)
                    continue;
            } else {
                unsigned char c;

                r = __libc16_dos_read(fd, &c, 1);
                if (!DOS_FAILED(r) && DOS_AX(r) == 1) {
                    if (c == LF) {
                        *out++ = LF;
                        break;
                    }
                    __libc16_fdpend[fd] = c;
                    *flags |= FD_PENDING;
                }
            }
        }
        *out++ = *p;
    }
    return out - (unsigned char *)buf;
}
