/* fopen - libc16 (docs/spec/stdio.md) */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <sys/stat.h>
#include "stdio16.h"

FILE * __cdecl fopen(const char *path, const char *mode)
{
    FILE *fp;
    int oflag, fd;
    unsigned char flag, seen = 0, c;

    switch (*mode++) {
    case 'r':
        oflag = O_RDONLY;
        flag = _IOREAD;
        break;
    case 'w':
        oflag = O_WRONLY | O_CREAT | O_TRUNC;
        flag = _IOWRT;
        break;
    case 'a':
        oflag = O_WRONLY | O_CREAT | O_APPEND;
        flag = _IOWRT;
        break;
    default:
        goto invalid;
    }
    oflag |= O_TEXT;
    /* Then "+", "b", both in either order, or neither: each at most once. */
    while ((c = *mode++) != '\0') {
        if (c == '+' && !(seen & 1)) {
            /* For reading and writing: O_RDWR in place of the access. */
            oflag = (oflag & ~(O_WRONLY | O_RDWR)) | O_RDWR;
            flag = _IOREAD | _IOWRT;
            seen |= 1;
        } else if (c == 'b' && !(seen & 2)) {
            oflag ^= O_TEXT | O_BINARY;
            seen |= 2;
        } else {
invalid:
            errno = EINVAL;
            return NULL;
        }
    }
    if ((fp = __libc16_stream()) == NULL)
        return NULL;
    /* The slot stays free until _flag is set. */
    if ((fd = open(path, oflag, S_IREAD | S_IWRITE)) == -1)
        return NULL;
    fp->_fd = fd;
    fp->_flag = flag;
    return fp;
}
