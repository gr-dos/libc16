/* setvbuf and setbuf, and when buffered output reaches DOS
 * (docs/spec/stdio.md, "Buffering")
 *
 * Whether output has reached DOS is seen two ways: tests/stdbuf/wrhook
 * counts the DOS writes to the stream's handle, and filelength on the
 * stream's own handle gives the length DOS has for the file. */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "check.h"

void __cdecl wrhook_install(void);
void __cdecl wrhook_remove(void);
void __cdecl wrhook_watch(int fd);
unsigned __cdecl wrhook_count(void);

static char buf[600];
static char sbuf[BUFSIZ];
static char got[1200];

/* The file's bytes, as they are on disk; its length, or -1. */
static int slurp(const char *name)
{
    int fd = open(name, O_RDONLY | O_BINARY), n;

    if (fd < 0)
        return -1;
    n = read(fd, got, sizeof got);
    close(fd);
    return n;
}

static int is(const char *name, const char *want)
{
    unsigned n = strlen(want);

    return slurp(name) == (int)n && memcmp(got, want, n) == 0;
}

static long len(FILE *fp)
{
    return filelength(fileno(fp));
}

static FILE *open_watched(const char *name, const char *mode)
{
    FILE *fp = fopen(name, mode);

    CHECK(fp != NULL);
    wrhook_watch(fileno(fp));
    return fp;
}

int main(void)
{
    FILE *fp, *fq, *in;
    void *blocks[64];
    unsigned i, nb;
    int c;

    wrhook_install();

    /* An invalid mode changes nothing: the stream stays unbuffered. */
    fp = open_watched("SV1.BIN", "wb");
    errno = 0;
    CHECK(setvbuf(fp, buf, 3, 16) != 0 && errno == EINVAL);
    errno = 0;
    CHECK(setvbuf(fp, buf, -1, 16) != 0 && errno == EINVAL);
    CHECK(fputc('a', fp) == 'a' && wrhook_count() == 1 && len(fp) == 1);
    /* So does a buffer of fewer than 2 bytes, or a size of 0 with one. */
    errno = 0;
    CHECK(setvbuf(fp, buf, _IOFBF, 1) != 0 && errno == EINVAL);
    errno = 0;
    CHECK(setvbuf(fp, buf, _IOLBF, 0) != 0 && errno == EINVAL);
    CHECK(fputc('b', fp) == 'b' && wrhook_count() == 2 && len(fp) == 2);

    /* Full buffering in the caller's buffer: nothing reaches DOS until
     * the buffer is full or flushed. */
    CHECK(setvbuf(fp, buf, _IOFBF, 16) == 0);
    CHECK(fputs("hello", fp) == 0 && wrhook_count() == 2 && len(fp) == 2);
    CHECK(ftell(fp) == 7);
    CHECK(fflush(fp) == 0 && wrhook_count() == 3 && len(fp) == 7);
    CHECK(fflush(fp) == 0 && wrhook_count() == 3);
    /* 40 bytes one at a time: two full buffers, 8 bytes pending */
    for (i = 0; i < 40; i++)
        CHECK(fputc('0' + i % 10, fp) == '0' + (int)(i % 10));
    CHECK(wrhook_count() == 5 && len(fp) == 39 && ftell(fp) == 47);
    CHECK(fclose(fp) == 0 && wrhook_count() == 6);
    CHECK(is("SV1.BIN", "abhello0123456789012345678901234567890123456789"));

    /* Text mode: the buffer holds the file's bytes, \n as \r\n. */
    fp = open_watched("SV2.TXT", "w");
    CHECK(setvbuf(fp, buf, _IOFBF, 8) == 0);
    CHECK(fputs("ab\ncd\n", fp) == 0 && wrhook_count() == 0 && ftell(fp) == 8);
    CHECK(fputc('x', fp) == 'x' && wrhook_count() == 1 && len(fp) == 8 && ftell(fp) == 9);
    /* a \n that does not fit whole is not split */
    CHECK(fputs("yzabcd\n", fp) == 0 && wrhook_count() == 2 && len(fp) == 15);
    CHECK(ftell(fp) == 17);
    CHECK(fclose(fp) == 0 && wrhook_count() == 3);
    CHECK(is("SV2.TXT", "ab\r\ncd\r\nxyzabcd\r\n"));
    fp = open_watched("SV2.TXT", "w");
    CHECK(setvbuf(fp, buf, _IOFBF, 4) == 0);
    CHECK(fputs("abc\n", fp) == 0 && wrhook_count() == 1 && len(fp) == 3 && ftell(fp) == 5);
    CHECK(fwrite("\n\n", 1, 2, fp) == 2 && wrhook_count() == 2 && ftell(fp) == 9);
    CHECK(fclose(fp) == 0 && is("SV2.TXT", "abc\r\n\r\n\r\n"));

    /* Line buffering: a call that writes a \n writes everything pending
     * before it returns, in one DOS write. */
    fp = open_watched("SV3.TXT", "w");
    CHECK(setvbuf(fp, buf, _IOLBF, 64) == 0);
    CHECK(fputs("one", fp) == 0 && wrhook_count() == 0);
    CHECK(fputs(" two\nthr", fp) == 0 && wrhook_count() == 1 && len(fp) == 12);
    CHECK(fputc('e', fp) == 'e' && wrhook_count() == 1 && ftell(fp) == 13);
    CHECK(fputc('\n', fp) == '\n' && wrhook_count() == 2 && len(fp) == 15);
    CHECK(fwrite("a\nb\nc", 1, 5, fp) == 5 && wrhook_count() == 3 && len(fp) == 22);
    /* a full buffer is written as well */
    for (i = 0; i < 70; i++)
        CHECK(fputc('z', fp) == 'z');
    CHECK(wrhook_count() == 4 && len(fp) == 22 + 64);
    CHECK(fclose(fp) == 0 && wrhook_count() == 5 && slurp("SV3.TXT") == 22 + 70);

    /* Changing the buffer, or to _IONBF, writes what is pending first. */
    fp = open_watched("SV4.BIN", "wb");
    CHECK(setvbuf(fp, buf, _IOFBF, 16) == 0 && fputs("abc", fp) == 0);
    CHECK(setvbuf(fp, buf + 100, _IOFBF, 32) == 0 && wrhook_count() == 1 && len(fp) == 3);
    CHECK(fputs("def", fp) == 0 && wrhook_count() == 1);
    CHECK(setvbuf(fp, NULL, _IONBF, 0) == 0 && wrhook_count() == 2 && len(fp) == 6);
    CHECK(fputc('g', fp) == 'g' && wrhook_count() == 3 && len(fp) == 7);
    /* _IONBF on an unbuffered stream: nothing to do */
    CHECK(setvbuf(fp, NULL, _IONBF, 0) == 0 && wrhook_count() == 3);
    /* setbuf: BUFSIZ bytes of the caller's, or none */
    setbuf(fp, sbuf);
    for (i = 0; i < BUFSIZ + 1; i++)
        fputc('h', fp);
    CHECK(wrhook_count() == 4 && len(fp) == 7 + BUFSIZ);
    setbuf(fp, NULL);
    CHECK(wrhook_count() == 5 && len(fp) == 8 + BUFSIZ);
    CHECK(fputc('i', fp) == 'i' && wrhook_count() == 6);
    CHECK(fclose(fp) == 0 && slurp("SV4.BIN") == 9 + BUFSIZ);

    /* A buffer from malloc: BUFSIZ bytes for a size of 0. */
    fp = open_watched("SV5.BIN", "wb");
    CHECK(setvbuf(fp, NULL, _IOFBF, 0) == 0);
    for (i = 0; i < 600; i++)
        fputc('m', fp);
    CHECK(wrhook_count() == 1 && len(fp) == BUFSIZ && ftell(fp) == 600);
    CHECK(setvbuf(fp, NULL, _IOFBF, 100) == 0 && wrhook_count() == 2 && len(fp) == 600);
    for (i = 0; i < 150; i++)
        fputc('n', fp);
    CHECK(wrhook_count() == 3 && len(fp) == 700);
    CHECK(fclose(fp) == 0 && slurp("SV5.BIN") == 750);

    /* A read stream is buffered already: _IOFBF succeeds and changes
     * nothing; the other two cannot be honoured. */
    in = fopen("SV1.BIN", "rb");
    CHECK(in != NULL && setvbuf(in, NULL, _IOFBF, 0) == 0 && setvbuf(in, buf, _IOFBF, 4) == 0);
    errno = 0;
    CHECK(setvbuf(in, NULL, _IOLBF, 0) != 0 && errno == EINVAL);
    errno = 0;
    CHECK(setvbuf(in, NULL, _IONBF, 0) != 0 && errno == EINVAL);
    CHECK(fgetc(in) == 'a' && fgetc(in) == 'b' && fgetc(in) == 'h');

    /* Input from any stream first writes the line-buffered streams'
     * pending output (a prompt), but not the fully buffered ones'. */
    fp = open_watched("SV6.TXT", "w");
    fq = fopen("SV7.TXT", "w");
    CHECK(fq != NULL && setvbuf(fp, buf, _IOLBF, 64) == 0 && setvbuf(fq, buf + 100, _IOFBF, 64) == 0);
    CHECK(fputs("name? ", fp) == 0 && fputs("full", fq) == 0 && wrhook_count() == 0);
    /* from in's buffer: no read, no flush */
    CHECK(fgetc(in) == 'e' && wrhook_count() == 0);
    CHECK(fread(got, 1, 2, in) == 2 && wrhook_count() == 0);
    /* a read refills the buffer */
    while ((c = fgetc(in)) != EOF && wrhook_count() == 0)
        ;
    CHECK(wrhook_count() == 1 && len(fp) == 6 && len(fq) == 0);
    CHECK(fclose(in) == 0);
    /* and so does fread, reading past its buffer */
    CHECK(fputs("again ", fp) == 0 && wrhook_count() == 1);
    in = fopen("SV5.BIN", "rb");
    CHECK(in != NULL && fread(got, 1, 700, in) == 700 && wrhook_count() == 2 && len(fp) == 12);
    CHECK(fclose(in) == 0);

    /* fflush(NULL) writes every stream's pending output. */
    CHECK(fputs("more", fp) == 0 && wrhook_count() == 2 && len(fq) == 0);
    CHECK(fflush(NULL) == 0 && wrhook_count() == 3 && len(fp) == 16 && len(fq) == 4);
    CHECK(fflush(NULL) == 0 && wrhook_count() == 3);
    CHECK(fclose(fp) == 0 && fclose(fq) == 0);
    CHECK(is("SV6.TXT", "name? again more") && is("SV7.TXT", "full"));

    /* fseek writes the pending output where it was written, first. */
    fp = open_watched("SV8.BIN", "wb");
    CHECK(setvbuf(fp, buf, _IOFBF, 16) == 0 && fputs("abcde", fp) == 0);
    errno = 0;
    CHECK(fseek(fp, 0L, 7) == -1 && errno == EINVAL && wrhook_count() == 0 && ftell(fp) == 5);
    CHECK(fseek(fp, -2L, SEEK_CUR) == 0 && wrhook_count() == 1 && len(fp) == 5 && ftell(fp) == 3);
    CHECK(fputc('X', fp) == 'X' && wrhook_count() == 1 && ftell(fp) == 4);
    CHECK(fseek(fp, 0L, SEEK_END) == 0 && wrhook_count() == 2 && ftell(fp) == 5);
    CHECK(fputs("fg", fp) == 0 && fseek(fp, 1L, SEEK_SET) == 0 && wrhook_count() == 3);
    CHECK(fputc('Y', fp) == 'Y' && fclose(fp) == 0);
    CHECK(is("SV8.BIN", "aYcXefg"));

    /* An append stream's flush goes at the end of the file, wherever
     * fseek put the stream. */
    fp = open_watched("SV8.BIN", "ab");
    CHECK(setvbuf(fp, buf, _IOFBF, 16) == 0 && fputs("hi", fp) == 0);
    CHECK(fseek(fp, 0L, SEEK_SET) == 0 && len(fp) == 9 && fputs("jk", fp) == 0);
    CHECK(fclose(fp) == 0 && is("SV8.BIN", "aYcXefghijk"));

    /* A stream from fdopen, and the handle's own output after a flush. */
    fp = fdopen(open("SV9.TXT", O_WRONLY | O_CREAT | O_TRUNC | O_TEXT, S_IREAD | S_IWRITE), "w");
    CHECK(fp != NULL && setvbuf(fp, buf, _IOFBF, 32) == 0);
    CHECK(fputs("stream\n", fp) == 0 && fflush(fp) == 0);
    CHECK(write(fileno(fp), "handle\n", 7) == 7 && fputs("end\n", fp) == 0);
    CHECK(fclose(fp) == 0 && is("SV9.TXT", "stream\r\nhandle\r\nend\r\n"));

    /* When malloc fails, setvbuf fails with ENOMEM and the stream keeps
     * what it had. */
    fp = open_watched("SVA.BIN", "wb");
    CHECK(setvbuf(fp, buf, _IOFBF, 16) == 0 && fputs("kept", fp) == 0);
    for (nb = 0; nb < 64 && (blocks[nb] = malloc(16384)) != NULL; nb++)
        ;
    CHECK(nb < 64);
    errno = 0;
    CHECK(setvbuf(fp, NULL, _IOFBF, 16384) != 0 && errno == ENOMEM);
    CHECK(wrhook_count() == 0 && ftell(fp) == 4 && fputs("still", fp) == 0 && wrhook_count() == 0);
    while (nb > 0)
        free(blocks[--nb]);
    CHECK(setvbuf(fp, NULL, _IOFBF, 16384) == 0 && wrhook_count() == 1 && len(fp) == 9);
    CHECK(fclose(fp) == 0 && is("SVA.BIN", "keptstill"));

    /* A closed stream */
    errno = 0;
    CHECK(setvbuf(fp, buf, _IOFBF, 16) != 0);

    wrhook_remove();
    return done();
}
