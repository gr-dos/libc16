/* unlink - libc16 (docs/spec/files.md) */

#include <errno.h>
#include <io.h>
#include <string.h>
#include "io16.h"

static unsigned long dos_unlink(const char *path);
#pragma aux dos_unlink = \
    "mov ah, 41h" \
    "int 21h" \
    "sbb dx, dx" \
    parm DS_DX value [dx ax] modify exact [ax dx];

int __cdecl unlink(const char *path)
{
    unsigned long r;

    if (strpbrk(path, "*?") != NULL) {
        errno = ENOENT;
        return -1;
    }
    r = dos_unlink(path);
    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    return 0;
}
