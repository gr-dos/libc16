/* Low-level I/O and errno (docs/spec/io.md) */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "check.h"

static char buf[700];

/* INT 21h 4301h: DOSBox-X ignores the attribute when it creates a file */
static unsigned set_attr(const char *path, unsigned attr);
#pragma aux set_attr = \
    "mov ax, 4301h" \
    "int 21h" \
    "sbb ax, ax" \
    parm [dx] [cx] value [ax];

/* tests/io/seekfail.asm: fail one INT 21h function 42h call */
void __cdecl seekfail_install(void);
void __cdecl seekfail_remove(void);
void __cdecl seekfail(int origin, unsigned skip);

/* Replaces NAME with the n bytes at data, in binary. */
static void put(const char *name, const char *data, unsigned n)
{
    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);

    CHECK(fd >= 0 && write(fd, data, n) == (int)n && close(fd) == 0);
}

/* The bytes of NAME, untranslated, in buf; returns how many. */
static int get(const char *name)
{
    int fd = open(name, O_RDONLY | O_BINARY), n;

    CHECK(fd >= 0);
    n = read(fd, buf, sizeof buf);
    CHECK(close(fd) == 0);
    return n;
}

int main(void)
{
    int fd, i;

    /* errors */
    errno = 0;
    CHECK(open("NOSUCH.TXT", O_RDONLY) == -1 && errno == ENOENT);
    CHECK(open("NODIR\\X.TXT", O_RDONLY) == -1 && errno == ENOENT);
    CHECK(open("T.TXT", 3) == -1 && errno == EINVAL);
    CHECK(open("T.TXT", O_RDONLY | O_TEXT | O_BINARY) == -1 && errno == EINVAL);
    CHECK(close(99) == -1 && errno == EBADF);
    CHECK(read(99, buf, 1) == -1 && errno == EBADF);
    CHECK(write(99, "x", 1) == -1 && errno == EBADF);
    errno = 0;
    CHECK(filelength(99) == -1L && errno == EBADF);
    /* handles outside the mode table, negative ones too */
    errno = 0;
    CHECK(read(-1, buf, 1) == -1 && errno == EBADF);
    errno = 0;
    CHECK(read(20, buf, 1) == -1 && errno == EBADF);
    errno = 0;
    CHECK(read(-1, buf, 0) == -1 && errno == EBADF);
    errno = 0;
    CHECK(write(-1, "x", 1) == -1 && errno == EBADF);
    errno = 0;
    CHECK(close(-1) == -1 && errno == EBADF);
    errno = 0;
    CHECK(setmode(-1, O_TEXT) == -1 && errno == EBADF);

    /* text mode is the default: \n is written as \r\n */
    fd = open("T.TXT", O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    CHECK(fd >= 0);
    CHECK(write(fd, "a\nb\n", 4) == 4);
    CHECK(write(fd, "", 0) == 0);
    CHECK(close(fd) == 0);
    CHECK(get("T.TXT") == 6 && memcmp(buf, "a\r\nb\r\n", 6) == 0);

    /* and read back as \n */
    fd = open("T.TXT", O_RDONLY);
    CHECK(filelength(fd) == 6);
    CHECK(read(fd, buf, sizeof buf) == 4 && memcmp(buf, "a\nb\n", 4) == 0);
    CHECK(read(fd, buf, sizeof buf) == 0);
    CHECK(read(fd, buf, 0) == 0);
    CHECK(close(fd) == 0);

    /* a write of 0 bytes does not truncate */
    fd = open("T.TXT", O_WRONLY | O_BINARY);
    CHECK(write(fd, buf, 0) == 0 && filelength(fd) == 6);
    CHECK(close(fd) == 0);

    /* a CR LF split across reads, a lone CR, a CR at the end */
    put("CR.TXT", "x\r\ny\rz\r", 7);
    fd = open("CR.TXT", O_RDONLY | O_TEXT);
    CHECK(read(fd, buf, 2) == 2 && memcmp(buf, "x\n", 2) == 0);
    CHECK(read(fd, buf, 2) == 2 && memcmp(buf, "y\r", 2) == 0);
    CHECK(filelength(fd) == 7);         /* keeps the byte read past "y\r" */
    CHECK(lseek(fd, 0L, SEEK_CUR) == 5);
    CHECK(read(fd, buf, 2) == 2 && memcmp(buf, "z\r", 2) == 0);
    CHECK(read(fd, buf, 2) == 0);
    CHECK(lseek(fd, 3L, SEEK_SET) == 3);
    CHECK(read(fd, buf, 1) == 1 && buf[0] == 'y');
    CHECK(read(fd, buf, 1) == 1 && buf[0] == '\r');
    CHECK(read(fd, buf, 10) == 2 && memcmp(buf, "z\r", 2) == 0);
    CHECK(close(fd) == 0);

    /* ^Z ends a text file */
    put("Z.TXT", "ab\x1a" "cd", 5);
    fd = open("Z.TXT", O_RDONLY);
    CHECK(read(fd, buf, 10) == 2 && memcmp(buf, "ab", 2) == 0);
    CHECK(read(fd, buf, 10) == 0);
    CHECK(lseek(fd, 0L, SEEK_CUR) == 2);
    CHECK(setmode(fd, O_BINARY) == O_TEXT);
    CHECK(read(fd, buf, 10) == 3 && memcmp(buf, "\x1a" "cd", 3) == 0);
    CHECK(close(fd) == 0);

    /* long text writes */
    memset(buf, '\n', 300);
    fd = open("LONG.TXT", O_WRONLY | O_CREAT | O_TRUNC | O_TEXT, S_IREAD | S_IWRITE);
    CHECK(write(fd, buf, 300) == 300);
    CHECK(close(fd) == 0);
    CHECK(get("LONG.TXT") == 600);
    for (i = 0; i < 600; i += 2)
        if (buf[i] != '\r' || buf[i + 1] != '\n')
            break;
    CHECK(i == 600);

    /* O_APPEND, O_EXCL, O_TRUNC */
    put("A.TXT", "12", 2);
    fd = open("A.TXT", O_WRONLY | O_APPEND | O_BINARY);
    CHECK(lseek(fd, 0L, SEEK_SET) == 0);
    CHECK(write(fd, "3", 1) == 1);
    CHECK(close(fd) == 0);
    CHECK(get("A.TXT") == 3 && memcmp(buf, "123", 3) == 0);
    CHECK(open("A.TXT", O_WRONLY | O_CREAT | O_EXCL, S_IREAD | S_IWRITE) == -1 && errno == EEXIST);
    fd = open("A.TXT", O_RDWR | O_TRUNC);
    CHECK(fd >= 0 && filelength(fd) == 0);
    CHECK(close(fd) == 0);

    /* lseek */
    put("S.TXT", "0123456789", 10);
    fd = open("S.TXT", O_RDWR | O_BINARY);
    CHECK(lseek(fd, 4L, SEEK_SET) == 4);
    CHECK(lseek(fd, 2L, SEEK_CUR) == 6);
    CHECK(lseek(fd, -1L, SEEK_END) == 9);
    CHECK(read(fd, buf, 5) == 1 && buf[0] == '9');
    CHECK(lseek(fd, -1L, SEEK_SET) == -1 && errno == EINVAL);
    errno = 0;
    CHECK(lseek(fd, -20L, SEEK_CUR) == -1 && errno == EINVAL);
    CHECK(lseek(fd, 0L, SEEK_CUR) == 10);
    CHECK(lseek(fd, 0L, 3) == -1 && errno == EINVAL);

    /* chsize keeps the position, extends with zeros, truncates */
    CHECK(lseek(fd, 2L, SEEK_SET) == 2);
    CHECK(chsize(fd, 1000L) == 0 && filelength(fd) == 1000);
    CHECK(lseek(fd, 0L, SEEK_CUR) == 2);
    CHECK(chsize(fd, 4L) == 0 && filelength(fd) == 4);
    CHECK(chsize(fd, -1L) == -1 && errno == EINVAL);
    CHECK(close(fd) == 0);
    put("S.TXT", "0123456789", 10);
    fd = open("S.TXT", O_RDWR | O_BINARY);
    CHECK(chsize(fd, 700L) == 0);
    CHECK(close(fd) == 0);
    CHECK(get("S.TXT") == 700 && memcmp(buf, "0123456789", 10) == 0);
    for (i = 10; i < 700 && buf[i] == 0; i++)
        ;
    CHECK(i == 700);
    fd = open("S.TXT", O_RDONLY | O_BINARY);
    CHECK(chsize(fd, 5L) == -1 && errno == EACCES);
    CHECK(close(fd) == 0);

    /* chsize when a seek fails */
    put("S.TXT", "0123456789", 10);
    fd = open("S.TXT", O_RDWR | O_BINARY);
    CHECK(lseek(fd, 3L, SEEK_SET) == 3);
    seekfail_install();
    /* finding the end: nothing changes */
    seekfail(SEEK_END, 0);
    errno = 0;
    CHECK(chsize(fd, 4L) == -1 && errno == EBADF);
    CHECK(filelength(fd) == 10 && lseek(fd, 0L, SEEK_CUR) == 3);
    /* the seek to the new end: nothing is truncated */
    seekfail(SEEK_SET, 0);
    errno = 0;
    CHECK(chsize(fd, 4L) == -1 && errno == EBADF);
    CHECK(filelength(fd) == 10 && lseek(fd, 0L, SEEK_CUR) == 3);
    /* restoring the position after shrinking, and after extending */
    seekfail(SEEK_SET, 1);
    errno = 0;
    CHECK(chsize(fd, 4L) == -1 && errno == EBADF && filelength(fd) == 4);
    CHECK(lseek(fd, 3L, SEEK_SET) == 3);
    seekfail(SEEK_SET, 0);
    errno = 0;
    CHECK(chsize(fd, 8L) == -1 && errno == EBADF && filelength(fd) == 8);
    /* and the hook is spent */
    CHECK(lseek(fd, 3L, SEEK_SET) == 3 && chsize(fd, 6L) == 0);
    CHECK(filelength(fd) == 6 && lseek(fd, 0L, SEEK_CUR) == 3);
    seekfail(-1, 0);
    seekfail_remove();
    CHECK(close(fd) == 0);

    /* access, and a read-only file */
    CHECK(access("S.TXT", 0) == 0 && access("S.TXT", 6) == 0);
    CHECK(access("NOSUCH.TXT", 0) == -1 && errno == ENOENT);
    CHECK(access("S.TXT", 1) == -1 && errno == EINVAL);
    fd = open("RO.TXT", O_RDONLY | O_CREAT, S_IREAD);
    CHECK(fd >= 0 && close(fd) == 0);
    CHECK(set_attr("RO.TXT", 1) == 0);
    CHECK(access("RO.TXT", 4) == 0);
    CHECK(access("RO.TXT", 2) == -1 && errno == EACCES);
    CHECK(open("RO.TXT", O_WRONLY) == -1 && errno == EACCES);

    /* setmode */
    CHECK(setmode(1, O_BINARY) == O_TEXT);
    CHECK(setmode(1, O_TEXT) == O_BINARY);
    CHECK(setmode(1, 5) == -1 && errno == EINVAL);
    CHECK(setmode(20, O_TEXT) == -1 && errno == EBADF);
    CHECK(write(1, "text\n", 5) == 5);
    return done();
}
