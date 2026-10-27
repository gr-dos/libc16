/* fwrite - libc16 (docs/spec/stdio.md) */

#include <io.h>
#include <stdio.h>

size_t __cdecl fwrite(const void *ptr, size_t size, size_t nmemb, FILE *fp)
{
    const unsigned char *p = ptr;
    unsigned want, put = 0, n;
    int r;

    if (size == 0 || nmemb == 0)
        return 0;
    /* No object is larger than 64 KiB, so neither is the request. */
    if (nmemb > 0xffffu / size)
        nmemb = 0xffffu / size;
    want = size * nmemb;
    if (!(fp->_flag & _IOWRT)) {
        fp->_flag |= _IOERR;
        return 0;
    }
    /* Output is unbuffered: the whole request goes to write, in pieces
     * whose counts write can return. */
    while (put < want) {
        n = want - put > 0x7fff ? 0x7fff : want - put;
        r = write(fp->_fd, p, n);
        if (r > 0) {
            p += r;
            put += r;
        }
        if (r != (int)n) {
            fp->_flag |= _IOERR;
            break;
        }
    }
    return put / size;
}
