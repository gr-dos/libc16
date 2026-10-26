/* Private to libc16: the stream table of fopen (docs/spec/stdio.md). */

#ifndef LIBC16_STDIO16_H
#define LIBC16_STDIO16_H

#include <stdio.h>

/* Streams fopen can open besides stdin, stdout and stderr; a slot is free
 * when its _flag is 0. */
#define STREAMS    (FOPEN_MAX - 3)
#define STREAM_BUF 128

extern FILE __libc16_streams[STREAMS];

#endif
