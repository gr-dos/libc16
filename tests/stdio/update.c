/* Update streams: fopen's and fdopen's r+, w+ and a+ (docs/spec/stdio.md,
 * "Update streams") */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "check.h"

#define CTLZ "\x1a"

static char buf[64];

/* Writes want to path, untranslated. */
static int make(const char *path, const char *want)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
    unsigned n = strlen(want);

    return fd >= 0 && write(fd, want, n) == (int)n && close(fd) == 0;
}

/* path holds exactly want, untranslated. */
static int file_is(const char *path, const char *want)
{
    int fd = open(path, O_RDONLY | O_BINARY);
    int n = fd < 0 ? -1 : read(fd, buf, sizeof buf);

    if (fd >= 0)
        close(fd);
    return n == (int)strlen(want) && memcmp(buf, want, n) == 0;
}

/* fopen refuses mode with EINVAL. */
static int bad_mode(const char *mode)
{
    errno = 0;
    return fopen("UP.BIN", mode) == NULL && errno == EINVAL;
}

int main(void)
{
    FILE *fp;
    int fd;

    /* r+ needs the file */
    unlink("NONE.BIN");
    errno = 0;
    CHECK(fopen("NONE.BIN", "r+") == NULL && errno == ENOENT);
    CHECK(fopen("NONE.BIN", "rb+") == NULL && errno == ENOENT);

    /* w+ creates; reading, writing and switching with fseek */
    fp = fopen("UP.BIN", "w+b");
    CHECK(fp != NULL && fputs("hello world", fp) == 0);
    CHECK(fgetc(fp) == EOF && feof(fp) && !ferror(fp));
    CHECK(fflush(fp) == 0 && fseek(fp, 0L, SEEK_SET) == 0 && !feof(fp));
    CHECK(fread(buf, 1, 5, fp) == 5 && memcmp(buf, "hello", 5) == 0);
    CHECK(fseek(fp, 0L, SEEK_CUR) == 0 && ftell(fp) == 5L);
    CHECK(fputs("XY", fp) == 0 && ftell(fp) == 7L);
    /* a read straight after a write continues after it */
    CHECK(fgetc(fp) == 'o' && fgets(buf, sizeof buf, fp) == buf && strcmp(buf, "rld") == 0);
    /* after a read that found the end, a write needs no fseek */
    CHECK(fgetc(fp) == EOF && feof(fp) && fputc('!', fp) == '!' && !ferror(fp));
    CHECK(ftell(fp) == 12L && fclose(fp) == 0);
    CHECK(file_is("UP.BIN", "helloXYorld!"));

    /* w+ truncates */
    fp = fopen("UP.BIN", "w+");
    CHECK(fp != NULL && fgetc(fp) == EOF && fclose(fp) == 0);
    CHECK(file_is("UP.BIN", ""));

    /* r+ keeps the file; rb+ and r+b are binary */
    CHECK(make("UP.BIN", "ab\r\ncd"));
    fp = fopen("UP.BIN", "r+");
    CHECK(fp != NULL && fclose(fp) == 0 && file_is("UP.BIN", "ab\r\ncd"));
    fp = fopen("UP.BIN", "rb+");
    CHECK(fp != NULL && fread(buf, 1, 4, fp) == 4 && memcmp(buf, "ab\r\n", 4) == 0);
    CHECK(fseek(fp, 1L, SEEK_SET) == 0 && fputc('B', fp) == 'B' && fclose(fp) == 0);
    fp = fopen("UP.BIN", "r+b");
    CHECK(fp != NULL && fgetc(fp) == 'a' && fgetc(fp) == 'B' && fgetc(fp) == '\r');
    CHECK(fclose(fp) == 0);

    /* r+ in text mode: positions count the file's bytes */
    CHECK(make("UP.TXT", "a\r\nb\r\nc"));
    fp = fopen("UP.TXT", "r+");
    CHECK(fp != NULL && fgets(buf, sizeof buf, fp) == buf && strcmp(buf, "a\n") == 0);
    CHECK(fseek(fp, 0L, SEEK_CUR) == 0 && ftell(fp) == 3L);
    CHECK(fputs("B\n", fp) == 0 && ftell(fp) == 6L);
    CHECK(fgetc(fp) == 'c' && fgetc(fp) == EOF && fclose(fp) == 0);
    CHECK(file_is("UP.TXT", "a\r\nB\r\nc"));

    /* a write after the ^Z that ended a text read overwrites it */
    CHECK(make("UP.TXT", "ab" CTLZ));
    fp = fopen("UP.TXT", "r+");
    CHECK(fp != NULL && fgetc(fp) == 'a' && fgetc(fp) == 'b' && fgetc(fp) == EOF);
    CHECK(feof(fp) && ftell(fp) == 2L && fputs("c\n", fp) == 0 && fclose(fp) == 0);
    CHECK(file_is("UP.TXT", "abc\r\n"));

    /* a+ reads from the start and writes at the end */
    CHECK(make("UP.BIN", "123"));
    fp = fopen("UP.BIN", "a+b");
    CHECK(fp != NULL && ftell(fp) == 0L && fgetc(fp) == '1');
    CHECK(fseek(fp, 0L, SEEK_SET) == 0 && fputc('x', fp) == 'x' && ftell(fp) == 4L);
    CHECK(fseek(fp, 1L, SEEK_SET) == 0 && fgetc(fp) == '2');
    CHECK(fseek(fp, 0L, SEEK_CUR) == 0 && fputs("yz", fp) == 0 && ftell(fp) == 6L);
    CHECK(fgetc(fp) == EOF && fclose(fp) == 0);
    CHECK(file_is("UP.BIN", "123xyz"));
    fp = fopen("UP.TXT", "a+");
    CHECK(fp != NULL && fputs("d\n", fp) == 0 && fseek(fp, 0L, SEEK_SET) == 0);
    CHECK(fgets(buf, sizeof buf, fp) == buf && strcmp(buf, "abc\n") == 0);
    CHECK(fgets(buf, sizeof buf, fp) == buf && strcmp(buf, "d\n") == 0 && fclose(fp) == 0);
    /* a+ creates */
    unlink("NONE.BIN");
    fp = fopen("NONE.BIN", "ab+");
    CHECK(fp != NULL && fgetc(fp) == EOF && fputc('n', fp) == 'n' && fclose(fp) == 0);
    CHECK(file_is("NONE.BIN", "n") && unlink("NONE.BIN") == 0);

    /* fdopen's update modes: no truncation, the handle's position */
    CHECK(make("UP.BIN", "0123"));
    fd = open("UP.BIN", O_RDWR | O_BINARY);
    CHECK(fd >= 0 && lseek(fd, 1L, SEEK_SET) == 1L);
    fp = fdopen(fd, "w+");
    CHECK(fp != NULL && fgetc(fp) == '1' && fseek(fp, 0L, SEEK_CUR) == 0);
    CHECK(fputc('X', fp) == 'X' && fgetc(fp) == '3' && fclose(fp) == 0);
    CHECK(file_is("UP.BIN", "01X3"));
    fd = open("UP.BIN", O_RDWR | O_BINARY);
    fp = fdopen(fd, "a+");
    CHECK(fp != NULL && fgetc(fp) == '0' && fseek(fp, 0L, SEEK_CUR) == 0);
    CHECK(fputc('4', fp) == '4' && fclose(fp) == 0);
    CHECK(file_is("UP.BIN", "01X34"));
    fd = open("UP.BIN", O_RDWR | O_BINARY);
    fp = fdopen(fd, "r+t");
    CHECK(fp != NULL && fseek(fp, 0L, SEEK_END) == 0 && fputc('\n', fp) == '\n');
    CHECK(fclose(fp) == 0 && file_is("UP.BIN", "01X34\r\n"));
    fd = open("UP.BIN", O_RDWR | O_BINARY);
    fp = fdopen(fd, "rb+");
    CHECK(fp != NULL && fseek(fp, -2L, SEEK_END) == 0 && fgetc(fp) == '\r');
    CHECK(fclose(fp) == 0);

    /* modes that are still invalid */
    CHECK(bad_mode("r++") && bad_mode("rb+b") && bad_mode("r+b+") && bad_mode("+r"));
    CHECK(bad_mode("r+x") && bad_mode("rbb") && bad_mode("w+t") && bad_mode("a+ "));
    fd = open("UP.BIN", O_RDWR | O_BINARY);
    errno = 0;
    CHECK(fdopen(fd, "r++") == NULL && errno == EINVAL);
    CHECK(fdopen(fd, "r+bt") == NULL && fdopen(fd, "rb+t") == NULL && fdopen(fd, "+r") == NULL);
    CHECK(close(fd) == 0);

    CHECK(unlink("UP.BIN") == 0 && unlink("UP.TXT") == 0);
    return done();
}
