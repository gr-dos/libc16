/* fputc - libc16 (docs/spec/stdio.md) */

#include <io.h>
#include <stdio.h>
#include "stdio16.h"

int __cdecl fputc(int c, FILE *fp)
{
    unsigned char ch = (unsigned char)c;

    if (fp->_flag & _IOWBUF) {
        /* A byte that is not \n goes into a buffer with room as it is. */
        if (ch != '\n' && fp->_ptr != fp->_base + fp->_bufsiz) {
            *fp->_ptr++ = ch;
            return ch;
        }
        return __libc16_bufput(fp, &ch, 1) == 1 ? ch : EOF;
    }
    if (!(fp->_flag & _IOWRT) || write(fp->_fd, &ch, 1) != 1) {
        fp->_flag |= _IOERR;
        return EOF;
    }
    return ch;
}
