/* filelength - libc16 (docs/spec/io.md) */

#include <io.h>
#include <stdio.h>

long __cdecl filelength(int fd)
{
    long here, end;

    if ((here = lseek(fd, 0L, SEEK_CUR)) == -1L)
        return -1L;
    end = lseek(fd, 0L, SEEK_END);
    lseek(fd, here, SEEK_SET);
    return end;
}
