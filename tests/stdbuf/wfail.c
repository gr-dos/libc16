/* Failed and short DOS writes of buffered output (docs/spec/stdio.md,
 * "Buffering", "Errors"). tests/stdbuf/wrhook makes a stream's writes
 * write fewer bytes than asked, as DOS does on a full disk, or fail with
 * a DOS error, writing nothing. */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include "check.h"

void __cdecl wrhook_install(void);
void __cdecl wrhook_remove(void);
void __cdecl wrhook_watch(int fd);
unsigned __cdecl wrhook_count(void);
void __cdecl wrhook_limit(unsigned skip, unsigned max);
void __cdecl wrhook_fail(unsigned skip, unsigned err);

static char buf[64];
static char big[100];
static char got[400];

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

static FILE *open_buffered(const char *name, const char *mode, int how, unsigned size)
{
    FILE *fp = fopen(name, mode);

    CHECK(fp != NULL && setvbuf(fp, buf, how, size) == 0);
    wrhook_watch(fileno(fp));
    return fp;
}

int main(void)
{
    FILE *fp;
    unsigned i;
    int fd;

    wrhook_install();
    for (i = 0; i < sizeof big; i++)
        big[i] = (char)('A' + i % 26);

    /* A short write: the earlier calls' bytes that DOS did not write stay
     * pending; the failing call's own are not left in the buffer. */
    fp = open_buffered("WF1.BIN", "wb", _IOFBF, 8);
    CHECK(fputs("abcdef", fp) == 0);
    wrhook_limit(0, 4);
    errno = 0;
    CHECK(fputs("ghij", fp) == EOF && ferror(fp) && errno == ENOSPC);
    CHECK(wrhook_count() == 1 && len(fp) == 4 && ftell(fp) == 6);
    /* writing again, once there is room */
    wrhook_watch(fileno(fp));
    clearerr(fp);
    CHECK(fflush(fp) == 0 && !ferror(fp) && wrhook_count() == 1 && len(fp) == 6);
    CHECK(fclose(fp) == 0 && is("WF1.BIN", "abcdef"));

    /* fwrite counts the elements DOS wrote whole: the 5 bytes written
     * are one element of 3 and part of the next. */
    fp = open_buffered("WF2.BIN", "wb", _IOFBF, 8);
    wrhook_limit(0, 5);
    errno = 0;
    CHECK(fwrite("123456789", 3, 3, fp) == 1 && ferror(fp) && errno == ENOSPC);
    CHECK(len(fp) == 5 && ftell(fp) == 5);
    wrhook_watch(fileno(fp));
    CHECK(fflush(fp) == 0 && wrhook_count() == 0);
    CHECK(fclose(fp) == 0 && is("WF2.BIN", "12345"));

    /* A disk that takes nothing more */
    fp = open_buffered("WF3.BIN", "wb", _IOFBF, 8);
    /* (one call of 8 would be written at once, as large as the buffer) */
    CHECK(fputs("1234", fp) == 0 && fputs("5678", fp) == 0);
    wrhook_limit(0, 0);
    errno = 0;
    CHECK(fputc('9', fp) == EOF && ferror(fp) && errno == ENOSPC && ftell(fp) == 8);
    errno = 0;
    CHECK(fflush(fp) == EOF && errno == ENOSPC && wrhook_count() == 2 && ftell(fp) == 8);
    wrhook_watch(fileno(fp));
    CHECK(fputc('9', fp) == '9' && wrhook_count() == 1 && len(fp) == 8);
    CHECK(fclose(fp) == 0 && is("WF3.BIN", "123456789"));

    /* A DOS error (5, access denied: EACCES) writes nothing */
    fp = open_buffered("WF4.BIN", "wb", _IOFBF, 4);
    CHECK(fputs("ab", fp) == 0 && fputs("cd", fp) == 0);
    wrhook_fail(0, 5);
    errno = 0;
    CHECK(fputs("ef", fp) == EOF && errno == EACCES && len(fp) == 0 && ftell(fp) == 4);
    errno = 0;
    CHECK(fseek(fp, 0L, SEEK_SET) == -1 && errno == EACCES && ftell(fp) == 4);
    wrhook_watch(fileno(fp));
    CHECK(fseek(fp, 0L, SEEK_END) == 0 && len(fp) == 4 && ftell(fp) == 4);
    CHECK(fclose(fp) == 0 && is("WF4.BIN", "abcd"));

    /* setvbuf that cannot write the old buffer's output fails and keeps
     * it */
    fp = open_buffered("WF5.BIN", "wb", _IOFBF, 8);
    CHECK(fputs("xyz", fp) == 0);
    wrhook_fail(0, 5);
    CHECK(setvbuf(fp, NULL, _IONBF, 0) != 0 && ftell(fp) == 3);
    wrhook_watch(fileno(fp));
    CHECK(setvbuf(fp, NULL, _IONBF, 0) == 0 && wrhook_count() == 1 && len(fp) == 3);
    CHECK(fclose(fp) == 0 && is("WF5.BIN", "xyz"));

    /* fclose that cannot write the pending output returns EOF, and still
     * closes the handle. */
    fp = open_buffered("WF6.BIN", "wb", _IOFBF, 8);
    fd = fileno(fp);
    CHECK(fputs("lost", fp) == 0);
    wrhook_fail(0, 5);
    CHECK(fclose(fp) == EOF);
    wrhook_watch(-1);
    errno = 0;
    CHECK(close(fd) == -1 && errno == EBADF);
    CHECK(slurp("WF6.BIN") == 0);

    /* Line buffering: the flush at the end of a call that fails fails the
     * call, whose bytes are not left pending. */
    fp = open_buffered("WF7.TXT", "w", _IOLBF, 16);
    CHECK(fputs("ab", fp) == 0);
    wrhook_fail(0, 5);
    CHECK(fputs("c\n", fp) == EOF && ftell(fp) == 2);
    wrhook_watch(fileno(fp));
    CHECK(fputs("d\n", fp) == 0 && wrhook_count() == 1);
    CHECK(fclose(fp) == 0 && is("WF7.TXT", "abd\r\n"));

    /* In text mode a \n counts as written when both bytes of its \r\n
     * are. */
    fp = open_buffered("WF8.TXT", "w", _IOLBF, 16);
    CHECK(fputs("abc", fp) == 0);
    wrhook_limit(0, 5);
    CHECK(fwrite("d\n", 1, 2, fp) == 1 && ftell(fp) == 5);
    wrhook_watch(fileno(fp));
    CHECK(fclose(fp) == 0 && wrhook_count() == 0 && is("WF8.TXT", "abcd\r"));

    /* A large binary request that DOS writes only in part: the buffer
     * is completed with 14 of its bytes and written, then 80 are written
     * straight from the request, of which DOS takes 60; the last 6 are
     * dropped */
    fp = open_buffered("WF9.BIN", "wb", _IOFBF, 16);
    CHECK(fputs("ab", fp) == 0);
    wrhook_limit(1, 60);
    errno = 0;
    CHECK(fwrite(big, 1, 100, fp) == 74 && ferror(fp) && errno == ENOSPC);
    CHECK(wrhook_count() == 2 && len(fp) == 76 && ftell(fp) == 76);
    CHECK(fclose(fp) == 0 && slurp("WF9.BIN") == 76 && memcmp(got + 2, big, 74) == 0);
    /* elements: 74 bytes are 7 whole elements of 10 */
    fp = open_buffered("WF9.BIN", "wb", _IOFBF, 16);
    CHECK(fputs("ab", fp) == 0);
    wrhook_limit(1, 60);
    CHECK(fwrite(big, 10, 10, fp) == 7 && ferror(fp));
    CHECK(fclose(fp) == 0 && slurp("WF9.BIN") == 76);
    /* and one whose pending output DOS does not take: nothing of it is
     * written */
    fp = open_buffered("WF9.BIN", "wb", _IOFBF, 16);
    CHECK(fputs("ab", fp) == 0);
    wrhook_fail(0, 5);
    CHECK(fwrite(big, 1, 100, fp) == 0 && wrhook_count() == 1 && ftell(fp) == 2);
    wrhook_watch(fileno(fp));
    CHECK(fclose(fp) == 0 && wrhook_count() == 1 && is("WF9.BIN", "ab"));

    /* fflush(NULL) reports a failure of any stream */
    fp = open_buffered("WFA.BIN", "wb", _IOFBF, 8);
    CHECK(fputs("q", fp) == 0);
    wrhook_fail(0, 5);
    CHECK(fflush(NULL) == EOF && ferror(fp));
    wrhook_watch(fileno(fp));
    CHECK(fflush(NULL) == 0 && fclose(fp) == 0 && is("WFA.BIN", "q"));

    wrhook_remove();
    return done();
}
