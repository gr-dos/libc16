/* fputc - libc16 (docs/spec/stdio.md) */

#include <io.h>
#include <stdio.h>

int __cdecl fputc(int c, FILE *fp)
{
    unsigned char ch = (unsigned char)c;

    if (!(fp->_flag & _IOWRT) || write(fp->_fd, &ch, 1) != 1) {
        fp->_flag |= _IOERR;
        return EOF;
    }
    return ch;
}
