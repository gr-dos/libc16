/* ftell - libc16 (docs/spec/stdio.md) */

#include <errno.h>
#include <stdio.h>
#include "io16.h"

#define SEEK_FAILED(r) ((unsigned)((r) >> 16) == 0xffff)

long __cdecl ftell(FILE *fp)
{
    int fd = fp->_fd;
    unsigned char *p, *q;
    unsigned long here, r;
    unsigned back = 0, err = 0;
    int text;
    unsigned char c;

    here = __libc16_dos_seek(fd, 0L, SEEK_CUR);
    if (SEEK_FAILED(here))
        return __libc16_doserr(DOS_AX(here));
    text = (unsigned)fd < FD_MAX && !(__libc16_fdflags[fd] & FD_BINARY);
    /* A byte kept by a text-mode read is before the file pointer. */
    if (text && (__libc16_fdflags[fd] & FD_PENDING))
        back = 1;
    if (fp->_cnt > 0) {
        p = fp->_ptr;
        q = p + fp->_cnt;
        if (!text) {
            back += fp->_cnt;
        } else {
            /* The buffered bytes end back bytes before the file pointer.
             * Going back over them, each stands for one byte of the file,
             * except a \n that read made from \r\n. read joins every \r\n
             * it reads whole, and the byte before an unread one is in the
             * same read (fgetc takes the first byte of each buffer it
             * fills), so the file's byte before the \n's own is read again
             * to see whether it is a \r. */
            do {
                back++;
                if (*--q == '\n' && here > back) {
                    c = 0;
                    r = __libc16_dos_seek(fd, (long)(here - back - 1), SEEK_SET);
                    if (SEEK_FAILED(r) || DOS_FAILED(r = __libc16_dos_read(fd, &c, 1))) {
                        err = DOS_AX(r);    /* never 0 */
                        break;
                    }
                    if (c == '\r')
                        back++;
                }
            } while (q != p);
            __libc16_dos_seek(fd, (long)here, SEEK_SET);
            if (err)
                return __libc16_doserr(err);
        }
    }
    /* A pushed-back byte is one byte back, and at 0 has no position. */
    if (fp->_flag & _IOUNGET)
        back++;
    if (here < back) {
        errno = EINVAL;
        return -1L;
    }
    return (long)(here - back);
}
