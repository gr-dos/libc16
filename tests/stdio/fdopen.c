/* fdopen (docs/spec/stdio.md) */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "check.h"

static char buf[64];

static int make(const char *path, const char *text)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);

    if (fd < 0 || write(fd, text, strlen(text)) != (int)strlen(text) || close(fd) != 0)
        return 0;
    return 1;
}

/* The file's contents are want */
static int file_is(const char *path, const char *want)
{
    int fd = open(path, O_RDONLY | O_BINARY);
    int n = fd < 0 ? -1 : read(fd, buf, sizeof buf);

    if (fd >= 0)
        close(fd);
    return n == (int)strlen(want) && memcmp(buf, want, n) == 0;
}

/* fdopen fails with err, and fd is still open in mode, at position pos */
static int refused(int fd, const char *mode, int err, int fmode, long pos)
{
    errno = 0;
    return fdopen(fd, mode) == NULL && errno == err
        && setmode(fd, fmode) == fmode && lseek(fd, 0L, SEEK_CUR) == pos;
}

int main(void)
{
    FILE *fp, *fps[5];
    int fd, i;

    CHECK(make("FD1.TXT", "abc\r\ndef"));

    /* r over a binary handle: reads from the handle's position, in its
     * mode; fileno; fclose closes the handle */
    fd = open("FD1.TXT", O_RDONLY | O_BINARY);
    CHECK(fd >= 0 && lseek(fd, 2L, SEEK_SET) == 2L);
    fp = fdopen(fd, "r");
    CHECK(fp != NULL && fileno(fp) == fd && !feof(fp) && !ferror(fp));
    CHECK(fgetc(fp) == 'c' && fgetc(fp) == '\r' && fgetc(fp) == '\n');
    CHECK(fputc('x', fp) == EOF && ferror(fp));        /* a read stream */
    CHECK(setmode(fd, O_BINARY) == O_BINARY);
    CHECK(fclose(fp) == 0);
    errno = 0;
    CHECK(close(fd) == -1 && errno == EBADF);

    /* r over a text handle keeps text mode */
    fd = open("FD1.TXT", O_RDONLY);
    fp = fdopen(fd, "r");
    CHECK(fp != NULL && fgets(buf, sizeof buf, fp) == buf && strcmp(buf, "abc\n") == 0);
    CHECK(setmode(fd, O_TEXT) == O_TEXT);
    CHECK(fclose(fp) == 0);

    /* rb makes a text handle binary */
    fd = open("FD1.TXT", O_RDONLY);
    fp = fdopen(fd, "rb");
    CHECK(fp != NULL && fgets(buf, sizeof buf, fp) == buf && strcmp(buf, "abc\r\n") == 0);
    CHECK(setmode(fd, O_BINARY) == O_BINARY);
    CHECK(fclose(fp) == 0);

    /* w does not truncate; wt makes a binary handle text */
    CHECK(make("FD2.TXT", "12345"));
    fd = open("FD2.TXT", O_WRONLY | O_BINARY);
    fp = fdopen(fd, "wt");
    CHECK(fp != NULL && fputs("a\n", fp) == 0 && !ferror(fp));
    CHECK(fgetc(fp) == EOF && ferror(fp));             /* a write stream */
    CHECK(setmode(fd, O_TEXT) == O_TEXT);
    CHECK(fclose(fp) == 0);
    CHECK(file_is("FD2.TXT", "a\r\n45"));

    /* w over a binary handle stays binary */
    fd = open("FD2.TXT", O_WRONLY | O_BINARY);
    fp = fdopen(fd, "w");
    CHECK(fp != NULL && fputs("b\n", fp) == 0);
    CHECK(fclose(fp) == 0);
    CHECK(file_is("FD2.TXT", "b\n\n45"));

    /* a appends after a seek to the start, as do later writes */
    fd = open("FD2.TXT", O_WRONLY | O_BINARY);
    CHECK(lseek(fd, 0L, SEEK_SET) == 0L);
    fp = fdopen(fd, "ab");
    CHECK(fp != NULL && fputc('Z', fp) == 'Z');
    CHECK(lseek(fd, 0L, SEEK_SET) == 0L && fputc('Y', fp) == 'Y');
    CHECK(fclose(fp) == 0);
    CHECK(file_is("FD2.TXT", "b\n\n45ZY"));

    /* over a handle from dup */
    fd = open("FD1.TXT", O_RDONLY | O_BINARY);
    fp = fdopen(dup(fd), "r");
    CHECK(fd >= 0 && fp != NULL && fileno(fp) != fd && fgetc(fp) == 'a');
    CHECK(fclose(fp) == 0 && close(fd) == 0);

    /* invalid modes, with the handle left as it was */
    fd = open("FD1.TXT", O_RDONLY | O_BINARY);
    CHECK(fd >= 0 && lseek(fd, 1L, SEEK_SET) == 1L);
    CHECK(refused(fd, "r+", EINVAL, O_BINARY, 1L));
    CHECK(refused(fd, "rb+", EINVAL, O_BINARY, 1L));
    CHECK(refused(fd, "rt+", EINVAL, O_BINARY, 1L));
    CHECK(refused(fd, "x", EINVAL, O_BINARY, 1L));
    CHECK(refused(fd, "", EINVAL, O_BINARY, 1L));
    CHECK(refused(fd, "rbt", EINVAL, O_BINARY, 1L));
    CHECK(refused(fd, "rtb", EINVAL, O_BINARY, 1L));

    /* a full table */
    for (i = 0; i < 5; i++)
        CHECK((fps[i] = fopen("FD1.TXT", "r")) != NULL);
    CHECK(refused(fd, "rt", EMFILE, O_BINARY, 1L));
    CHECK(refused(fd, "a", EMFILE, O_BINARY, 1L));
    CHECK(read(fd, buf, 2) == 2 && memcmp(buf, "bc", 2) == 0);
    for (i = 0; i < 5; i++)
        CHECK(fclose(fps[i]) == 0);
    CHECK(close(fd) == 0);

    /* handles that are not open */
    errno = 0;
    CHECK(fdopen(fd, "r") == NULL && errno == EBADF);
    errno = 0;
    CHECK(fdopen(-1, "r") == NULL && errno == EBADF);
    errno = 0;
    CHECK(fdopen(20, "r") == NULL && errno == EBADF);

    /* the table is usable again, by fdopen and by fopen */
    fd = open("FD1.TXT", O_RDONLY | O_BINARY);
    fp = fdopen(fd, "r");
    fps[0] = fopen("FD1.TXT", "rb");
    CHECK(fp != NULL && fps[0] != NULL && fp != fps[0]);
    CHECK(fgetc(fp) == 'a' && fgetc(fps[0]) == 'a');
    CHECK(fclose(fp) == 0 && fclose(fps[0]) == 0);

    /* standard output, through a stream over a duplicate of handle 1 */
    fp = fdopen(dup(1), "w");
    CHECK(fp != NULL && fputs("through fdopen\n", fp) == 0);
    CHECK(fclose(fp) == 0);
    return done();
}
