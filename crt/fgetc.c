/* fgetc - libc16 (docs/spec/stdio.md) */

#include <io.h>
#include <stdio.h>

int __cdecl fgetc(FILE *fp)
{
    int n;

    if (fp->_flag & _IOUNGET) {
        fp->_flag &= ~_IOUNGET;
        return fp->_hold;
    }
    if (fp->_cnt <= 0) {
        if (!(fp->_flag & _IOREAD)) {
            fp->_flag |= _IOERR;
            return EOF;
        }
        if (fp->_flag & _IOEOF)
            return EOF;
        n = read(fp->_fd, fp->_base, fp->_bufsiz);
        if (n <= 0) {
            fp->_flag |= n == 0 ? _IOEOF : _IOERR;
            return EOF;
        }
        fp->_ptr = fp->_base;
        fp->_cnt = n;
    }
    fp->_cnt--;
    return *fp->_ptr++;
}
