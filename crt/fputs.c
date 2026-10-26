/* fputs - libc16 (docs/spec/stdio.md) */

#include <io.h>
#include <stdio.h>
#include <string.h>

int __cdecl fputs(const char *s, FILE *fp)
{
    unsigned n = strlen(s);

    if (!(fp->_flag & _IOWRT) || write(fp->_fd, s, n) != (int)n) {
        fp->_flag |= _IOERR;
        return EOF;
    }
    return 0;
}
