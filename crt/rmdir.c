/* rmdir - libc16 (docs/spec/files.md) */

#include <direct.h>
#include <errno.h>
#include "io16.h"

static unsigned long dos_rmdir(const char *path);
#pragma aux dos_rmdir = \
    "mov ah, 3ah" \
    "int 21h" \
    "sbb dx, dx" \
    parm DS_DX value [dx ax] modify exact [ax dx];

int __cdecl rmdir(const char *path)
{
    const char *s = path;
    unsigned long r;
    char c;

    /* A wildcard names no directory ("Names with wildcards") */
    while ((c = *s++) != '\0') {
        if (c == '*' || c == '?') {
            errno = ENOENT;
            return -1;
        }
    }
    r = dos_rmdir(path);
    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    return 0;
}
