/* fclose - libc16 (docs/spec/stdio.md) */

#include <io.h>
#include <stdio.h>

int __cdecl fclose(FILE *fp)
{
    int rc = close(fp->_fd);

    fp->_flag = 0;
    fp->_cnt = 0;
    return rc == 0 ? 0 : EOF;
}
