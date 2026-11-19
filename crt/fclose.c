/* fclose - libc16 (docs/spec/stdio.md) */

#include <io.h>
#include <stdio.h>
#include "stdio16.h"

int __cdecl fclose(FILE *fp)
{
    /* The pending output, then the buffer, then the handle. */
    int rc = (fp->_flag & _IOWBUF) ? __libc16_bufctl(fp, BUF_CLOSE) : 0;

    if (close(fp->_fd) != 0)
        rc = EOF;
    fp->_flag = 0;
    fp->_cnt = 0;
    return rc == 0 ? 0 : EOF;
}
