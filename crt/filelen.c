/* filelength - libc16 (docs/spec/io.md) */

#include <io.h>
#include <stdio.h>
#include "io16.h"

#define SEEK_FAILED(r) ((unsigned)((r) >> 16) == 0xffff)

/* DOS seeks only, not lseek: the DOS file pointer is put back exactly, so
 * a byte kept by a text-mode read stays valid. */
long __cdecl filelength(int fd)
{
    unsigned long here, end;

    here = __libc16_dos_seek(fd, 0L, SEEK_CUR);
    if (SEEK_FAILED(here))
        return __libc16_doserr(DOS_AX(here));
    end = __libc16_dos_seek(fd, 0L, SEEK_END);
    __libc16_dos_seek(fd, (long)here, SEEK_SET);
    if (SEEK_FAILED(end))
        return __libc16_doserr(DOS_AX(end));
    return (long)end;
}
