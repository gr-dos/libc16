/* Private to libc16: the stream table of fopen and fdopen, the seek
 * helper of fseek and ftell, and the hooks of buffered output
 * (docs/spec/stdio.md). */

#ifndef LIBC16_STDIO16_H
#define LIBC16_STDIO16_H

#include <stdio.h>

/* Streams fopen and fdopen can open besides stdin, stdout and stderr; a
 * slot is free when its _flag is 0. The table and its buffers are their
 * own member, crt/streams.c, which both link. */
#define STREAMS    (FOPEN_MAX - 3)
#define STREAM_BUF 128

extern FILE __libc16_streams[STREAMS];

/* A free slot of the table with its buffer set up and empty, still free
 * until the caller sets _fd and _flag; or a null pointer with errno
 * EMFILE if every slot is in use. */
FILE *__libc16_stream(void);

/* After a DOS error err in fseek or ftell: moves fp's handle back to
 * here, or if that fails too, discards the buffer, the pushed-back byte
 * and a byte kept by read. Sets errno from err and returns -1 (ftell). */
int __libc16_seekback(FILE *fp, unsigned long here, unsigned err);

/* Buffered output (docs/spec/stdio.md, "Buffering"). The engine,
 * crt/bufio.c, is linked only by setvbuf and setbuf, which set these two
 * pointers before any stream has _IOWBUF; they are null until then. The
 * pointers are their own member, crt/bufhook.c, so that the stream
 * functions reach the engine without linking it.
 *
 * __libc16_bufput: writes n bytes to fp, which has _IOWBUF; returns how
 * many of them were written or are pending, n unless the call failed
 * (the error indicator and errno are then set, and none of the call's
 * bytes is left pending).
 *
 * __libc16_bufctl: BUF_FLUSH writes fp's pending output (0 if fp has no
 * buffer); BUF_ALL that of every stream; BUF_LINE that of every
 * line-buffered stream; BUF_CLOSE flushes fp, then gives up its buffer
 * and makes it unbuffered. Returns 0, or EOF if a write failed. */
#define BUF_FLUSH 0
#define BUF_ALL   1
#define BUF_LINE  2
#define BUF_CLOSE 3

extern unsigned (*__libc16_bufput)(FILE *fp, const void *p, unsigned n);
extern int (*__libc16_bufctl)(FILE *fp, int how);

/* Calls __libc16_bufctl(fp, how) if the engine is linked; 0 if not. */
#define BUFCTL(fp, how) (__libc16_bufctl != NULL ? __libc16_bufctl(fp, how) : 0)

/* The engine's own: gives fp the buffer buf of size bytes in mode
 * (_IOFBF or _IOLBF), or none (_IONBF), after flushing it. If release is
 * not null, buf is from malloc and release frees it, when the stream
 * gives it up or now if the call fails. Returns 0, or -1 with errno. */
int __libc16_bufset(FILE *fp, char *buf, int mode, unsigned size,
                    void (__cdecl *release)(void *));

/* Set by crt/bufexit.asm once the termination table has flushed the
 * streams: no stream gets a buffer after that. */
extern unsigned char __libc16_bufdone;

#endif
