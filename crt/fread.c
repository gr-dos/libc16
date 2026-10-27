/* fread - libc16 (docs/spec/stdio.md) */

#include <io.h>
#include <stdio.h>
#include <string.h>

size_t __cdecl fread(void *ptr, size_t size, size_t nmemb, FILE *fp)
{
    unsigned char *p = ptr;
    unsigned want, got = 0, n;
    int r;

    if (size == 0 || nmemb == 0)
        return 0;
    /* No object is larger than 64 KiB, so neither is the request. */
    if (nmemb > 0xffffu / size)
        nmemb = 0xffffu / size;
    want = size * nmemb;
    if (!(fp->_flag & _IOREAD)) {
        fp->_flag |= _IOERR;
        return 0;
    }
    if (fp->_flag & _IOUNGET) {
        fp->_flag &= ~_IOUNGET;
        *p++ = fp->_hold;
        got++;
    }
    if (got < want && fp->_cnt > 0) {
        n = want - got < (unsigned)fp->_cnt ? want - got : (unsigned)fp->_cnt;
        memcpy(p, fp->_ptr, n);
        fp->_ptr += n;
        fp->_cnt -= n;
        p += n;
        got += n;
    }
    /* The rest straight from the file, in pieces read can return. */
    while (got < want && !(fp->_flag & _IOEOF)) {
        n = want - got > 0x7fff ? 0x7fff : want - got;
        r = read(fp->_fd, p, n);
        if (r <= 0) {
            fp->_flag |= r == 0 ? _IOEOF : _IOERR;
            break;
        }
        p += r;
        got += r;
    }
    return got / size;
}
