/* DOS error codes to errno - libc16 (docs/spec/io.md) */

#include <errno.h>
#include "io16.h"

/* errno for DOS errors 0 to 18 */
static const unsigned char map[] = {
    EACCES, EINVAL, ENOENT, ENOENT, EMFILE, EACCES, EBADF, ENOMEM,
    ENOMEM, EINVAL, E2BIG, ENOEXEC, EINVAL, EINVAL, EACCES, ENOENT,
    EACCES, EXDEV, ENOENT
};

int __libc16_doserr(unsigned code)
{
    if (code < sizeof map)
        errno = map[code];
    else if (code == 80)
        errno = EEXIST;
    else
        errno = EACCES;
    return -1;
}
