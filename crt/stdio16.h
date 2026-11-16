/* Private to libc16: the stream table of fopen (docs/spec/stdio.md). */

#ifndef LIBC16_STDIO16_H
#define LIBC16_STDIO16_H

#include <stdio.h>

/* Streams fopen can open besides stdin, stdout and stderr; a slot is free
 * when its _flag is 0. */
#define STREAMS    (FOPEN_MAX - 3)
#define STREAM_BUF 128

extern FILE __libc16_streams[STREAMS];

/* After a DOS error err in fseek or ftell: moves fp's handle back to
 * here, or if that fails too, discards the buffer, the pushed-back byte
 * and a byte kept by read. Sets errno from err and returns -1 (ftell). */
int __libc16_seekback(FILE *fp, unsigned long here, unsigned err);

#endif
