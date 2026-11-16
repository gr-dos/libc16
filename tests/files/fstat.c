/* fstat (docs/spec/files.md), with TZ unset */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include "check.h"

/* 2026-07-01 12:00:00 UTC */
#define SUMMER 1782907200L

static void del_file(const char *path);
#pragma aux del_file = \
    "mov ah, 41h" \
    "int 21h" \
    parm T_DS_DX modify exact [ax];

static int make(const char *name, const char *text)
{
    int fd;
    unsigned n = strlen(text);

    del_file(name);
    fd = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
    CHECK(fd >= 0 && write(fd, text, n) == (int)n);
    return fd;
}

int main(void)
{
    struct stat st, st2;
    struct utimbuf t;
    char buf[8];
    int fd;

    /* a file being written */
    fd = make("FS.TXT", "0123456789");
    memset(&st, 0x55, sizeof st);
    CHECK(fstat(fd, &st) == 0);
    CHECK(st.st_mode == (S_IFREG | 0666) && st.st_size == 10);
    CHECK(st.st_dev == 2 && st.st_rdev == 2 && st.st_ino == 0 && st.st_nlink == 1);
    CHECK(st.st_uid == 0 && st.st_gid == 0);
    CHECK(st.st_atime == st.st_mtime && st.st_ctime == st.st_mtime);
    CHECK(close(fd) == 0);

    /* the time is DOS's, as stat reports it */
    t.actime = t.modtime = SUMMER;
    CHECK(utime("FS.TXT", &t) == 0);
    fd = open("FS.TXT", O_RDONLY | O_BINARY);
    CHECK(fd >= 0 && fstat(fd, &st) == 0 && stat("FS.TXT", &st2) == 0);
    CHECK(st.st_mtime == SUMMER && st2.st_mtime == SUMMER);
    CHECK(st.st_atime == SUMMER && st.st_ctime == SUMMER && st.st_size == 10);

    /* the position is unchanged */
    CHECK(lseek(fd, 3L, SEEK_SET) == 3 && fstat(fd, &st) == 0 && st.st_size == 10);
    CHECK(read(fd, buf, 1) == 1 && buf[0] == '3');
    CHECK(close(fd) == 0);

    /* a byte kept by a text-mode read is kept */
    fd = make("FK.TXT", "ab\rc");
    CHECK(close(fd) == 0);
    fd = open("FK.TXT", O_RDONLY | O_TEXT);
    CHECK(fd >= 0 && read(fd, buf, 3) == 3 && memcmp(buf, "ab\r", 3) == 0);
    CHECK(fstat(fd, &st) == 0 && st.st_size == 4);
    CHECK(read(fd, buf, 3) == 1 && buf[0] == 'c' && read(fd, buf, 3) == 0);
    CHECK(close(fd) == 0);

    /* standard error is the console; standard output is redirected to a
     * file */
    memset(&st, 0x55, sizeof st);
    CHECK(fstat(2, &st) == 0);
    CHECK(st.st_mode == (S_IFCHR | 0666) && st.st_size == 0);
    CHECK(st.st_dev == 2 && st.st_rdev == 2 && st.st_mtime == 0 && st.st_atime == 0 && st.st_ctime == 0);
    CHECK(st.st_nlink == 1 && st.st_ino == 0);
    CHECK(fstat(1, &st) == 0 && (st.st_mode & S_IFMT) == S_IFREG && st.st_dev == 2);

    /* closed and invalid handles */
    errno = 0;
    CHECK(fstat(fd, &st) == -1 && errno == EBADF);
    errno = 0;
    CHECK(fstat(99, &st) == -1 && errno == EBADF);
    errno = 0;
    CHECK(fstat(-1, &st) == -1 && errno == EBADF);
    return done();
}
