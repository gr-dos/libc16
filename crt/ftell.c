/* ftell - libc16 (docs/spec/stdio.md) */

#include <errno.h>
#include <stdio.h>
#include "io16.h"
#include "stdio16.h"

#define SEEK_FAILED(r) ((unsigned)((r) >> 16) == 0xffff)

long __cdecl ftell(FILE *fp)
{
    /* Each buffered or pushed-back byte came from at most 2 bytes. */
    unsigned char raw[2 * (STREAM_BUF + 1)];
    unsigned long r;
    long here, pos, start;
    unsigned back, want, i;
    int fd = fp->_fd;
    int text = 0;

    r = __libc16_dos_seek(fd, 0L, SEEK_CUR);
    if (SEEK_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    here = (long)r;
    pos = here;
    if ((unsigned)fd < FD_MAX) {
        /* A byte kept by a text-mode read is past the DOS file pointer. */
        if (__libc16_fdflags[fd] & FD_PENDING)
            pos--;
        text = !(__libc16_fdflags[fd] & FD_BINARY);
    }
    back = fp->_cnt > 0 ? (unsigned)fp->_cnt : 0;
    if (fp->_flag & _IOUNGET)
        back++;
    if (back == 0)
        return pos;
    if (!text) {
        if (pos < (long)back) {
            errno = EINVAL;
            return -1L;
        }
        return pos - back;
    }

    /* Text mode: read the bytes before pos untranslated and count back
     * over them, a \r\n being one buffered \n. */
    want = 2 * back;
    if (want > sizeof raw) {
        errno = EINVAL;
        return -1L;
    }
    start = pos - want;
    if (start < 0) {
        want = (unsigned)pos;
        start = 0;
    }
    r = __libc16_dos_seek(fd, start, SEEK_SET);
    if (!SEEK_FAILED(r))
        r = __libc16_dos_read(fd, raw, want);
    __libc16_dos_seek(fd, here, SEEK_SET);
    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    if (DOS_AX(r) != want) {
        errno = EINVAL;
        return -1L;
    }
    i = want;
    while (back > 0) {
        if (i == 0) {
            errno = EINVAL;
            return -1L;
        }
        if (raw[i - 1] == '\n' && i >= 2 && raw[i - 2] == '\r')
            i -= 2;
        else
            i--;
        back--;
    }
    return start + i;
}
