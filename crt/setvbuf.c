/* setvbuf - libc16 (docs/spec/stdio.md, "Buffering") */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdio16.h"

int __cdecl setvbuf(FILE *fp, char *buf, int mode, size_t size)
{
    void (__cdecl *release)(void *) = NULL;

    /* A buffer of its own only for a write stream that would get one:
     * not for an update stream, whose one buffer is its input buffer. */
    if (buf == NULL && (mode == _IOFBF || mode == _IOLBF)
            && (fp->_flag & (_IOREAD | _IOWRT)) == _IOWRT
            && !__libc16_bufdone) {
        if (size == 0)
            size = BUFSIZ;
        if (size >= 2) {
            if ((buf = malloc(size)) == NULL) {
                errno = ENOMEM;
                return -1;
            }
            release = free;
        }
    }
    return __libc16_bufset(fp, buf, mode, size, release);
}
