/* setmode - libc16 (docs/spec/io.md) */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include "io16.h"

int __cdecl setmode(int fd, int mode)
{
    unsigned char *flags;
    int old;

    if ((unsigned)fd >= FD_MAX) {
        errno = EBADF;
        return -1;
    }
    flags = &__libc16_fdflags[fd];
    if (mode != O_TEXT && mode != O_BINARY) {
        errno = EINVAL;
        return -1;
    }
    old = (*flags & FD_BINARY) ? O_BINARY : O_TEXT;
    if (mode == O_BINARY)
        *flags |= FD_BINARY;
    else
        *flags &= ~FD_BINARY;
    return old;
}
