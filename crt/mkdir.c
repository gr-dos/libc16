/* mkdir - libc16 (docs/spec/files.md) */

#include <direct.h>
#include "io16.h"

static unsigned long dos_mkdir(const char *path);
#pragma aux dos_mkdir = \
    "mov ah, 39h" \
    "int 21h" \
    "sbb dx, dx" \
    parm DS_DX value [dx ax] modify exact [ax dx];

int __cdecl mkdir(const char *path)
{
    unsigned long r = dos_mkdir(path);

    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    return 0;
}
