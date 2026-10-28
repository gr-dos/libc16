/* rename, unlink and remove (docs/spec/files.md) */

#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "check.h"

static void make(const char *name, const char *text)
{
    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);

    CHECK(fd >= 0 && write(fd, text, strlen(text)) == (int)strlen(text) && close(fd) == 0);
}

/* 1 if name holds exactly text */
static int holds(const char *name, const char *text)
{
    char buf[16];
    int fd = open(name, O_RDONLY | O_BINARY);
    int n;

    if (fd < 0)
        return 0;
    n = read(fd, buf, sizeof buf);
    close(fd);
    return n == (int)strlen(text) && memcmp(buf, text, n) == 0;
}

static int exists(const char *name)
{
    return access(name, 0) == 0;
}

int main(void)
{
    /* leftovers from an earlier run */
    chmod("RO.TXT", S_IREAD | S_IWRITE);
    unlink("A.TXT");
    unlink("B.TXT");
    unlink("C.TXT");
    unlink("RO.TXT");
    unlink("SUB\\MOVED.TXT");
    mkdir("SUB");

    /* rename in place, and the contents follow */
    make("A.TXT", "alpha");
    errno = 0;
    CHECK(rename("A.TXT", "B.TXT") == 0 && errno == 0);
    CHECK(!exists("A.TXT") && holds("B.TXT", "alpha"));

    /* into another directory on the same drive */
    CHECK(rename("B.TXT", "SUB\\MOVED.TXT") == 0);
    CHECK(!exists("B.TXT") && holds("SUB\\MOVED.TXT", "alpha"));
    CHECK(rename("SUB\\MOVED.TXT", "B.TXT") == 0 && holds("B.TXT", "alpha"));

    /* onto an existing file: refused, both unchanged */
    make("C.TXT", "gamma");
    errno = 0;
    CHECK(rename("B.TXT", "C.TXT") == -1 && errno == EACCES);
    CHECK(holds("B.TXT", "alpha") && holds("C.TXT", "gamma"));

    /* a missing file, a missing directory */
    errno = 0;
    CHECK(rename("NONE.TXT", "D.TXT") == -1 && errno == ENOENT);
    errno = 0;
    CHECK(rename("B.TXT", "NODIR\\D.TXT") == -1 && errno == ENOENT);
    CHECK(holds("B.TXT", "alpha"));

    /* another drive */
    errno = 0;
    CHECK(rename("B.TXT", "Z:\\B.TXT") == -1 && errno == EXDEV);

    /* wildcards in either name */
    errno = 0;
    CHECK(rename("B.T?T", "D.TXT") == -1 && errno == ENOENT);
    errno = 0;
    CHECK(rename("B.TXT", "*.BAK") == -1 && errno == ENOENT);
    CHECK(holds("B.TXT", "alpha"));

    /* unlink and remove */
    CHECK(unlink("B.TXT") == 0 && !exists("B.TXT"));
    CHECK(remove("C.TXT") == 0 && !exists("C.TXT"));
    errno = 0;
    CHECK(unlink("B.TXT") == -1 && errno == ENOENT);
    errno = 0;
    CHECK(remove("B.TXT") == -1 && errno == ENOENT);
    make("RO.TXT", "ro");
    CHECK(chmod("RO.TXT", S_IREAD) == 0);
    errno = 0;
    CHECK(unlink("RO.TXT") == -1 && errno == EACCES && holds("RO.TXT", "ro"));
    errno = 0;
    CHECK(remove("RO.TXT") == -1 && errno == EACCES);
    CHECK(chmod("RO.TXT", S_IREAD | S_IWRITE) == 0 && remove("RO.TXT") == 0);
    errno = 0;
    CHECK(unlink("SUB") == -1 && errno == EACCES && exists("SUB"));
    make("W.TXT", "w");
    errno = 0;
    CHECK(unlink("W.*") == -1 && errno == ENOENT && exists("W.TXT"));
    CHECK(remove("W.TXT") == 0);

    return done();
}
