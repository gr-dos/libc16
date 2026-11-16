/* chdir - libc16 (docs/spec/files.md) */

#include <direct.h>
#include "io16.h"

static unsigned long dos_chdir(const char *path);
#pragma aux dos_chdir = \
    "mov ah, 3bh" \
    "int 21h" \
    "sbb dx, dx" \
    parm DS_DX value [dx ax] modify exact [ax dx];

int __cdecl chdir(const char *path)
{
    unsigned long r = dos_chdir(path);

    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    return 0;
}
