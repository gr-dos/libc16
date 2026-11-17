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
    unsigned char flag;

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
        errno = EINVAL;
        return NULL;
    }
    oflag |= O_TEXT;
    if (*mode == 'b') {
        oflag ^= O_TEXT | O_BINARY;
        mode++;
    }
    if (*mode != '\0') {
        errno = EINVAL;
        return NULL;
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
