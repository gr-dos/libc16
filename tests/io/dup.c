/* dup and dup2 (docs/spec/io.md) */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "check.h"

static char buf[64];

/* The file's contents are want */
static int file_is(const char *path, const char *want)
{
    int fd = open(path, O_RDONLY | O_BINARY);
    int n = fd < 0 ? -1 : read(fd, buf, sizeof buf);

    if (fd >= 0)
        close(fd);
    return n == (int)strlen(want) && memcmp(buf, want, n) == 0;
}

static int make(const char *path, const char *text)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);

    if (fd < 0 || write(fd, text, strlen(text)) != (int)strlen(text) || close(fd) != 0)
        return 0;
    return 1;
}

int main(void)
{
    int fd, fd2, fd3, save, i, n;
    int fds[20];

    /* dup of a binary handle: same file, shared position, same mode */
    fd = open("DUP1.TXT", O_RDWR | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
    CHECK(fd >= 0);
    fd2 = dup(fd);
    CHECK(fd2 >= 0 && fd2 != fd);
    CHECK(write(fd, "ab\n", 3) == 3);
    CHECK(lseek(fd2, 0L, SEEK_CUR) == 3L);
    CHECK(write(fd2, "c\n", 2) == 2);           /* binary: no \r */
    CHECK(lseek(fd, 0L, SEEK_CUR) == 5L);
    CHECK(setmode(fd2, O_BINARY) == O_BINARY);
    /* closing either leaves the other open */
    CHECK(close(fd) == 0);
    CHECK(lseek(fd2, 0L, SEEK_SET) == 0L && read(fd2, buf, 8) == 5);
    CHECK(close(fd2) == 0);
    CHECK(file_is("DUP1.TXT", "ab\nc\n"));

    /* dup of a text handle */
    fd = open("DUP1.TXT", O_WRONLY | O_TRUNC);
    fd2 = dup(fd);
    CHECK(fd >= 0 && fd2 >= 0 && write(fd2, "t\n", 2) == 2);
    CHECK(setmode(fd2, O_BINARY) == O_TEXT && setmode(fd, O_TEXT) == O_TEXT);
    CHECK(close(fd2) == 0 && close(fd) == 0);
    CHECK(file_is("DUP1.TXT", "t\r\n"));

    /* dup of an appending handle appends */
    fd = open("DUP1.TXT", O_WRONLY | O_APPEND | O_BINARY);
    fd2 = dup(fd);
    CHECK(fd >= 0 && fd2 >= 0);
    CHECK(lseek(fd2, 0L, SEEK_SET) == 0L && write(fd2, "x", 1) == 1);
    CHECK(close(fd2) == 0 && close(fd) == 0);
    CHECK(file_is("DUP1.TXT", "t\r\nx"));

    /* a byte kept by a text-mode read stays with the old handle */
    CHECK(make("DUP2.TXT", "a\rb"));
    fd = open("DUP2.TXT", O_RDONLY);
    CHECK(fd >= 0 && read(fd, buf, 2) == 2 && memcmp(buf, "a\r", 2) == 0);
    fd2 = dup(fd);
    CHECK(fd2 >= 0 && read(fd2, buf, 4) == 0);
    CHECK(read(fd, buf, 4) == 1 && buf[0] == 'b');
    CHECK(close(fd2) == 0 && close(fd) == 0);

    /* a closed handle */
    errno = 0;
    CHECK(dup(fd) == -1 && errno == EBADF);
    errno = 0;
    CHECK(dup(-1) == -1 && errno == EBADF);

    /* no free handle */
    for (n = 0; n < 20; n++)
        if ((fds[n] = open("DUP1.TXT", O_RDONLY)) < 0)
            break;
    CHECK(n > 0 && n < 20 && errno == EMFILE);
    errno = 0;
    CHECK(dup(0) == -1 && errno == EMFILE);
    for (i = 0; i < n; i++)
        close(fds[i]);

    /* dup2 onto an open handle: its file is closed, it gets fd's mode,
     * and its kept byte is discarded */
    CHECK(make("DUP2.TXT", "a\rb"));
    fd2 = open("DUP2.TXT", O_RDONLY);
    CHECK(fd2 >= 0 && read(fd2, buf, 2) == 2);         /* keeps 'b' */
    fd = open("DUP3.TXT", O_RDWR | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
    CHECK(fd >= 0 && write(fd, "123", 3) == 3);
    CHECK(dup2(fd, fd2) == 0);
    CHECK(setmode(fd2, O_BINARY) == O_BINARY);
    CHECK(lseek(fd2, 0L, SEEK_CUR) == 3L);
    CHECK(lseek(fd, 0L, SEEK_SET) == 0L);
    CHECK(read(fd2, buf, 8) == 3 && memcmp(buf, "123", 3) == 0);   /* no 'b' */
    CHECK(write(fd2, "\n", 1) == 1);
    CHECK(lseek(fd2, 0L, SEEK_SET) == 0L && read(fd2, buf, 8) == 4 && memcmp(buf, "123\n", 4) == 0);
    CHECK(close(fd2) == 0);
    CHECK(lseek(fd, 0L, SEEK_CUR) == 4L);      /* fd is still open */

    /* onto a closed handle */
    CHECK(dup2(fd, fd2) == 0);
    CHECK(lseek(fd2, 1L, SEEK_SET) == 1L && lseek(fd, 0L, SEEK_CUR) == 1L);
    CHECK(close(fd2) == 0);

    /* onto itself */
    CHECK(dup2(fd, fd) == 0);
    CHECK(lseek(fd, 0L, SEEK_CUR) == 1L);
    CHECK(close(fd) == 0);
    CHECK(file_is("DUP3.TXT", "123\n"));

    /* from a closed handle, and to handles outside 0 to 19: the second
     * handle is left alone */
    fd3 = open("DUP2.TXT", O_RDONLY | O_BINARY);
    fd = dup(fd3);
    CHECK(fd3 >= 0 && fd >= 0 && close(fd) == 0);
    errno = 0;
    CHECK(dup2(fd, fd3) == -1 && errno == EBADF);
    CHECK(read(fd3, buf, 8) == 3 && memcmp(buf, "a\rb", 3) == 0);
    errno = 0;
    CHECK(dup2(fd3, -1) == -1 && errno == EBADF);
    errno = 0;
    CHECK(dup2(fd3, 20) == -1 && errno == EBADF);
    errno = 0;
    CHECK(dup2(fd3, 25) == -1 && errno == EBADF);
    CHECK(close(fd3) == 0);

    /* redirect standard output to a file and back */
    save = dup(1);
    CHECK(save >= 0);
    fd = open("DUP4.TXT", O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
    CHECK(fd >= 0 && dup2(fd, 1) == 0 && close(fd) == 0);
    CHECK(write(1, "to file", 7) == 7);
    CHECK(dup2(save, 1) == 0 && close(save) == 0);
    CHECK(file_is("DUP4.TXT", "to file"));
    CHECK(setmode(1, O_TEXT) == O_TEXT);
    t_puts("back on stdout\r\n");
    return done();
}
