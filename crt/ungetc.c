/* ungetc - libc16 (docs/spec/stdio.md) */

#include <stdio.h>

int __cdecl ungetc(int c, FILE *fp)
{
    if (c == EOF || (fp->_flag & _IOUNGET))
        return EOF;
    fp->_hold = (unsigned char)c;
    fp->_flag = (fp->_flag & ~_IOEOF) | _IOUNGET;
    return fp->_hold;
}
