/* rename, unlink and remove (docs/spec/files.md) */

#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "check.h"

/* INT 21h 41h and 3Ah, and 4301h to clear the attributes first, so that a
 * run starts without what an earlier one left */
static void del_file(const char *path);
#pragma aux del_file = \
    "push dx" \
    "xor cx, cx" \
    "mov ax, 4301h" \
    "int 21h" \
    "pop dx" \
    "mov ah, 41h" \
    "int 21h" \
    parm T_DS_DX modify exact [ax cx];

static void del_dir(const char *path);
#pragma aux del_dir = \
    "mov ah, 3ah" \
    "int 21h" \
    parm T_DS_DX modify exact [ax];

/* Creates name holding text. */
static void make(const char *name, const char *text)
{
    int fd;
    unsigned n = strlen(text);

    del_file(name);
    fd = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
    CHECK(fd >= 0 && write(fd, text, n) == (int)n && close(fd) == 0);
}

/* 1 if name exists and holds exactly text */
static int holds(const char *name, const char *text)
{
    char buf[32];
    int fd, n;

    fd = open(name, O_RDONLY | O_BINARY);
    if (fd < 0)
        return 0;
    n = read(fd, buf, sizeof buf);
    close(fd);
    return n == (int)strlen(text) && memcmp(buf, text, n) == 0;
}

static int missing(const char *name)
{
    return access(name, 0) == -1 && errno == ENOENT;
}

/* rename fails with err and changes nothing */
static int refused(const char *from, const char *to, int err)
{
    errno = 0;
    return rename(from, to) == -1 && errno == err;
}

static void renames(void)
{
    make("RA.TXT", "alpha");
    CHECK(rename("RA.TXT", "RB.TXT") == 0);
    CHECK(missing("RA.TXT") && holds("RB.TXT", "alpha"));

    /* into a directory and back, with drive letters */
    CHECK(mkdir("RD") == 0);
    CHECK(rename("RB.TXT", "RD\\RC.TXT") == 0);
    CHECK(missing("RB.TXT") && holds("RD\\RC.TXT", "alpha"));
    CHECK(rename("c:RD\\RC.TXT", "C:\\RA.TXT") == 0);
    CHECK(missing("RD\\RC.TXT") && holds("RA.TXT", "alpha"));
    CHECK(rename("C:RA.TXT", "RB.TXT") == 0 && holds("RB.TXT", "alpha"));
    CHECK(rename("RB.TXT", "c:RA.TXT") == 0 && holds("RA.TXT", "alpha"));

    /* onto an existing file or directory: nothing changes */
    make("RE.TXT", "beta");
    CHECK(refused("RA.TXT", "RE.TXT", EACCES));
    CHECK(holds("RA.TXT", "alpha") && holds("RE.TXT", "beta"));
    CHECK(refused("RA.TXT", "RD", EACCES));
    CHECK(holds("RA.TXT", "alpha") && access("RD", 0) == 0);

    /* a missing file, a missing directory */
    CHECK(refused("RX.TXT", "RY.TXT", ENOENT) && missing("RY.TXT"));
    CHECK(refused("RA.TXT", "RZ\\RA.TXT", ENOENT) && holds("RA.TXT", "alpha"));
    CHECK(refused("RZ\\RA.TXT", "RY.TXT", ENOENT) && missing("RY.TXT"));

    /* another drive, named either way round */
    CHECK(refused("RA.TXT", "Z:RA.TXT", EXDEV) && holds("RA.TXT", "alpha"));
    CHECK(refused("C:RA.TXT", "z:\\RA.TXT", EXDEV) && holds("RA.TXT", "alpha"));
    CHECK(refused("Z:RA.TXT", "RA.TXT", EXDEV) && holds("RA.TXT", "alpha"));

    /* wildcards in either name */
    CHECK(refused("R*.TXT", "RY.TXT", ENOENT) && missing("RY.TXT"));
    CHECK(refused("R?.TXT", "RY.TXT", ENOENT) && missing("RY.TXT"));
    CHECK(refused("RA.TXT", "R?.TXT", ENOENT) && holds("RA.TXT", "alpha"));
    CHECK(refused("RA.TXT", "RD\\*.TXT", ENOENT) && holds("RA.TXT", "alpha"));
    CHECK(holds("RE.TXT", "beta"));
}

static void deletes(void)
{
    /* unlink */
    CHECK(unlink("RA.TXT") == 0 && missing("RA.TXT"));
    errno = 0;
    CHECK(unlink("RA.TXT") == -1 && errno == ENOENT);

    make("RO.TXT", "gamma");
    CHECK(chmod("RO.TXT", S_IREAD) == 0);
    errno = 0;
    CHECK(unlink("RO.TXT") == -1 && errno == EACCES && holds("RO.TXT", "gamma"));
    errno = 0;
    CHECK(remove("RO.TXT") == -1 && errno == EACCES && holds("RO.TXT", "gamma"));
    CHECK(chmod("RO.TXT", S_IREAD | S_IWRITE) == 0 && unlink("RO.TXT") == 0);
    CHECK(missing("RO.TXT"));

    errno = 0;
    CHECK(unlink("RD") == -1 && errno == EACCES && access("RD", 0) == 0);
    errno = 0;
    CHECK(remove("RD") == -1 && errno == EACCES && access("RD", 0) == 0);

    errno = 0;
    CHECK(unlink("R*.TXT") == -1 && errno == ENOENT && holds("RE.TXT", "beta"));
    errno = 0;
    CHECK(remove("RE.TX?") == -1 && errno == ENOENT && holds("RE.TXT", "beta"));

    /* remove */
    CHECK(remove("RE.TXT") == 0 && missing("RE.TXT"));
    errno = 0;
    CHECK(remove("RE.TXT") == -1 && errno == ENOENT);
    errno = 0;
    CHECK(remove("RZ\\RE.TXT") == -1 && errno == ENOENT);
}

int main(void)
{
    del_file("RA.TXT");
    del_file("RB.TXT");
    del_file("RE.TXT");
    del_file("RO.TXT");
    del_file("RY.TXT");
    del_file("RD\\RC.TXT");
    del_file("RD\\RA.TXT");
    del_dir("RD");

    renames();
    deletes();
    del_dir("RD");
    return done();
}
