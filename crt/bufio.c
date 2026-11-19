/* Buffered output - libc16 (docs/spec/stdio.md, "Buffering")
 *
 * The engine behind setvbuf and setbuf. A write stream with _IOWBUF keeps
 * its pending output from _base to _ptr, as the bytes are to be in the
 * file: in text mode each \n is stored as \r\n when it is accepted, so a
 * flush is one DOS write of the buffer (INT 21h 40h, after a seek to the
 * end for an append handle), and DOS's count says exactly which bytes
 * reached the file. Bytes DOS did not write stay pending, moved to the
 * start of the buffer, except those of the call that failed.
 *
 * The streams with a buffer are listed in bufd, so that fflush(NULL),
 * the spawn functions and termination (crt/bufexit.asm) find them
 * without linking the stream table. */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "io16.h"
#include "stdio16.h"

static FILE *bufd[FOPEN_MAX];

/* The release function of the buffers from malloc (setvbuf's free). */
static void (__cdecl *release_fn)(void *);

static int is_text(int fd)
{
    return (unsigned)fd < FD_MAX && !(__libc16_fdflags[fd] & FD_BINARY);
}

/* Writes n bytes from p to fp's handle in one DOS call, at the end of
 * the file for an append handle. Returns the count DOS wrote; if it is
 * less than n, sets errno and the error indicator. */
static unsigned put(FILE *fp, const unsigned char *p, unsigned n)
{
    int fd = fp->_fd;
    unsigned long r;

    if (n == 0)
        return 0;
    if ((unsigned)fd < FD_MAX && (__libc16_fdflags[fd] & FD_APPEND))
        __libc16_dos_seek(fd, 0L, SEEK_END);
    r = __libc16_dos_write(fd, p, n);
    if (DOS_FAILED(r)) {
        __libc16_doserr(DOS_AX(r));
        r = 0;
    } else if (DOS_AX(r) != n) {
        errno = ENOSPC;
    }
    if (DOS_AX(r) != n)
        fp->_flag |= _IOERR;
    return DOS_AX(r);
}

/* Writes fp's pending output. Returns the count written; what DOS did not
 * write is moved to the start of the buffer and stays pending. */
static unsigned flush(FILE *fp, int *failed)
{
    unsigned n = (unsigned)(fp->_ptr - fp->_base), k, i;

    k = put(fp, fp->_base, n);
    if (k != n) {
        for (i = 0; i < n - k; i++)
            fp->_base[i] = fp->_base[k + i];
        *failed = 1;
    }
    fp->_ptr = fp->_base + (n - k);
    return k;
}

/* Counts, not end pointers, bound the copies: a buffer or a request may
 * end at the end of its segment, where a pointer past it wraps. */
static unsigned bufput(FILE *fp, const void *src, unsigned n)
{
    const unsigned char *s = src;
    unsigned char *d, *mark;
    int text = is_text(fp->_fd), failed = 0, line = 0;
    unsigned left = n, done = 0, earlier, room, k, w, need;
    unsigned char c;

    if (!text && n >= fp->_bufsiz) {
        /* Too large to be worth copying: what is pending, then the
         * request, each in one write. */
        earlier = (unsigned)(fp->_ptr - fp->_base);
        k = flush(fp, &failed);
        if (failed)
            goto fail;
        return put(fp, s, n);
    }
    if (!text && (fp->_flag & _IOLBUF))
        for (k = 0; k < n; k++)
            if (s[k] == '\n')
                line = 1;
    mark = fp->_ptr;    /* where this call's bytes start */
    for (;;) {
        d = fp->_ptr;
        room = fp->_bufsiz - (unsigned)(d - fp->_base);
        if (text) {
            while (left != 0) {
                if ((c = *s) == '\n') {
                    if (room < 2)
                        break;
                    *d++ = '\r';
                    room--;
                    line = 1;
                } else if (room == 0) {
                    break;
                }
                *d++ = c;
                room--;
                s++;
                left--;
            }
        } else {
            k = left < room ? left : room;
            memcpy(d, s, k);
            d += k;
            s += k;
            left -= k;
        }
        fp->_ptr = d;
        if (left == 0)
            break;
        /* The buffer is full. */
        earlier = (unsigned)(mark - fp->_base);
        k = flush(fp, &failed);
        if (failed)
            goto fail;
        done = n - left;
        mark = fp->_base;
    }
    if (line && (fp->_flag & _IOLBUF)) {
        earlier = (unsigned)(mark - fp->_base);
        k = flush(fp, &failed);
        if (failed)
            goto fail;
    }
    return n;

fail:
    /* k bytes were written: first the earlier calls' pending bytes, then
     * w of this call's, from source byte done on. This call's bytes that
     * are left are dropped; the earlier ones stay pending. */
    w = k > earlier ? k - earlier : 0;
    s = (const unsigned char *)src + done;
    while (done < n) {
        need = text && *s == '\n' ? 2 : 1;
        if (w < need)
            break;
        w -= need;
        done++;
        s++;
    }
    fp->_ptr = fp->_base + (k < earlier ? earlier - k : 0);
    return done;
}

static int bufctl(FILE *fp, int how)
{
    FILE **q;
    int failed = 0;

    if (fp == NULL) {
        for (q = bufd; q != bufd + FOPEN_MAX; q++)
            if (*q != NULL && (how != BUF_LINE || ((*q)->_flag & _IOLBUF))
                    && (*q)->_ptr != (*q)->_base)
                flush(*q, &failed);
        return failed ? EOF : 0;
    }
    if (!(fp->_flag & _IOWBUF))
        return 0;
    flush(fp, &failed);
    if (how == BUF_CLOSE) {
        if ((fp->_flag & _IOMYBUF) && release_fn != NULL)
            release_fn(fp->_base);
        fp->_flag &= ~(_IOWBUF | _IOLBUF | _IOMYBUF);
        fp->_ptr = fp->_base = NULL;
        fp->_bufsiz = 0;
        for (q = bufd; q != bufd + FOPEN_MAX; q++)
            if (*q == fp)
                *q = NULL;
    }
    return failed ? EOF : 0;
}

/* Termination (crt/bufexit.asm): writes every stream's pending output and
 * makes the streams unbuffered, so that output after this, from an
 * atexit function or another routine, goes straight to DOS in order. */
void __libc16_buffini(void)
{
    FILE **q;
    int failed = 0;

    for (q = bufd; q != bufd + FOPEN_MAX; q++) {
        if (*q != NULL) {
            flush(*q, &failed);
            (*q)->_flag &= ~(_IOWBUF | _IOLBUF);
            *q = NULL;
        }
    }
}

int __libc16_bufset(FILE *fp, char *buf, int mode, unsigned size,
                    void (__cdecl *release)(void *))
{
    FILE **q, **slot = NULL;

    if ((unsigned)mode > _IONBF) {
        errno = EINVAL;
        goto fail;
    }
    if (!(fp->_flag & (_IOREAD | _IOWRT))) {
        errno = EBADF;
        goto fail;
    }
    if (fp->_flag & _IOREAD) {
        /* Input is always buffered, by the stream's own buffer. */
        if (mode == _IOFBF)
            goto unused;
        errno = EINVAL;
        goto fail;
    }
    if (mode != _IONBF && (__libc16_bufdone || buf == NULL || size < 2)) {
        errno = EINVAL;
        goto fail;
    }
    __libc16_bufput = bufput;
    __libc16_bufctl = bufctl;
    /* The old buffer's output first; if that fails, it stays. */
    if (bufctl(fp, BUF_FLUSH) != 0)
        goto fail;
    bufctl(fp, BUF_CLOSE);
    if (mode == _IONBF)
        goto unused;
    for (q = bufd; q != bufd + FOPEN_MAX; q++)
        if (*q == NULL)
            slot = q;
    /* There are at most FOPEN_MAX streams, and fp is on no list now. */
    *slot = fp;
    fp->_ptr = fp->_base = (unsigned char *)buf;
    fp->_cnt = 0;
    fp->_bufsiz = size;
    fp->_flag |= _IOWBUF;
    if (mode == _IOLBF)
        fp->_flag |= _IOLBUF;
    if (release != NULL) {
        fp->_flag |= _IOMYBUF;
        release_fn = release;
    }
    return 0;

unused:
    if (release != NULL)
        release(buf);
    return 0;

fail:
    if (release != NULL)
        release(buf);
    return -1;
}
