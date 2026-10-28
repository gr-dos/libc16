/* rename - libc16 (docs/spec/files.md) */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "io16.h"

/* INT 21h 56h takes the old name in DS:DX and the new one in ES:DI. */
static unsigned long dos_rename(const char *oldname, const char *newname);
#if defined(__COMPACT__) || defined(__LARGE__)
#pragma aux dos_rename = \
    "mov ah, 56h" \
    "int 21h" \
    "sbb dx, dx" \
    parm [ds dx] [es di] value [dx ax] modify exact [ax dx];
#else
#pragma aux dos_rename = \
    "push ds" \
    "pop es" \
    "mov ah, 56h" \
    "int 21h" \
    "sbb dx, dx" \
    parm [dx] [di] value [dx ax] modify exact [ax dx es];
#endif

int __cdecl rename(const char *oldname, const char *newname)
{
    unsigned long r;

    if (strpbrk(oldname, "*?") != NULL || strpbrk(newname, "*?") != NULL) {
        errno = ENOENT;
        return -1;
    }
    r = dos_rename(oldname, newname);
    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    return 0;
}
