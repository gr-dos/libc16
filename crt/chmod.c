/* chmod - libc16 (docs/spec/files.md) */

#include <dos.h>
#include <io.h>
#include <sys/stat.h>
#include "io16.h"

int __cdecl chmod(const char *path, int pmode)
{
    unsigned long r = __libc16_dos_getattr(path);
    unsigned attr;

    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    attr = DOS_AX(r) & ~_A_RDONLY;
    if (!(pmode & S_IWRITE))
        attr |= _A_RDONLY;
    r = __libc16_dos_setattr(path, attr);
    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    return 0;
}
