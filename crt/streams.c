/* The stream table of fopen and fdopen, and its buffers - libc16
 * (docs/spec/stdio.md). Its own member, so that fopen does not link fdopen
 * and fdopen does not link fopen or open. */

#include <errno.h>
#include <stdio.h>
#include "stdio16.h"

FILE __libc16_streams[STREAMS];
static unsigned char bufs[STREAMS][STREAM_BUF];

FILE *__libc16_stream(void)
{
    FILE *fp = __libc16_streams;
    unsigned char *buf = bufs[0];

    while (fp->_flag != 0) {
        if (fp == &__libc16_streams[STREAMS - 1]) {
            errno = EMFILE;
            return NULL;
        }
        fp++;
        buf += STREAM_BUF;
    }
    fp->_ptr = fp->_base = buf;
    fp->_cnt = 0;
    fp->_bufsiz = STREAM_BUF;
    return fp;
}
