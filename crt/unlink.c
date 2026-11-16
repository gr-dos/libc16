/* unlink - libc16 (docs/spec/files.md) */

#include <dos.h>
#include <errno.h>
#include <io.h>
#include "io16.h"

static unsigned long dos_delete(const char *path);
#pragma aux dos_delete = \
    "mov ah, 41h" \
    "int 21h" \
    "sbb dx, dx" \
    parm DS_DX value [dx ax] modify exact [ax dx];

int __cdecl unlink(const char *path)
{
    const char *s = path;
    unsigned long r;
    char c;

    /* DOS would delete every file a wildcard matches */
    while ((c = *s++) != '\0') {
        if (c == '*' || c == '?') {
            errno = ENOENT;
            return -1;
        }
    }
    /* Not every DOS refuses a read-only file or a directory itself
     * (DOSBox-X deletes a read-only file, and reports a directory as not
     * found), so the attributes are checked first. */
    r = __libc16_dos_getattr(path);
    if (!DOS_FAILED(r)) {
        if (DOS_AX(r) & (_A_RDONLY | _A_SUBDIR)) {
            errno = EACCES;
            return -1;
        }
        r = dos_delete(path);
    }
    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    return 0;
}
