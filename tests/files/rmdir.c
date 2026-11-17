/* rmdir (docs/spec/files.md) */

#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <sys/stat.h>
#include "check.h"

/* INT 21h 3Ah and 41h, so that a run starts without what an earlier one
 * left */
static void del_dir(const char *path);
#pragma aux del_dir = \
    "mov ah, 3ah" \
    "int 21h" \
    parm T_DS_DX modify exact [ax];

static void del_file(const char *path);
#pragma aux del_file = \
    "mov ah, 41h" \
    "int 21h" \
    parm T_DS_DX modify exact [ax];

static char buf[80];

/* path is a directory: chdir into it and back works */
static int is_dir(const char *path)
{
    return chdir(path) == 0 && chdir("\\") == 0;
}

/* rmdir(path) fails with err, leaving path a directory */
static int refused(const char *path, int err)
{
    errno = 0;
    return rmdir(path) == -1 && errno == err && is_dir(path);
}

static void cleanup(void)
{
    del_file("RD\\FULL\\F.TXT");
    del_file("RD\\F.TXT");
    del_dir("RD\\FULL");
    del_dir("RD\\SUB");
    del_dir("RD\\E1");
    del_dir("RD\\E2");
    del_dir("RD\\CUR");
    del_dir("RD\\W1");
    del_dir("RD");
}

int main(void)
{
    int fd;

    cleanup();
    CHECK(mkdir("RD") == 0);

    /* empty directories, by several spellings */
    CHECK(mkdir("RD\\E1") == 0 && rmdir("RD\\E1") == 0);
    errno = 0;
    CHECK(chdir("RD\\E1") == -1 && errno == ENOENT);
    CHECK(mkdir("RD\\E2") == 0 && rmdir("c:\\rd\\e2") == 0);
    CHECK(chdir("RD\\E2") == -1);
    CHECK(mkdir("RD\\SUB") == 0 && chdir("RD") == 0 && rmdir("SUB") == 0 && chdir("\\") == 0);
    CHECK(chdir("RD\\SUB") == -1);

    /* not empty */
    CHECK(mkdir("RD\\FULL") == 0);
    fd = open("RD\\FULL\\F.TXT", O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    CHECK(fd >= 0 && close(fd) == 0);
    CHECK(refused("RD\\FULL", EACCES));
    CHECK(access("RD\\FULL\\F.TXT", 0) == 0);

    /* the current directory, and the root */
    CHECK(mkdir("RD\\CUR") == 0 && chdir("RD\\CUR") == 0);
    errno = 0;
    CHECK(rmdir("\\RD\\CUR") == -1 && errno == EACCES);
    errno = 0;
    CHECK(rmdir(".") == -1 && errno == EACCES);
    CHECK(getcwd(buf, sizeof buf) == buf && strcmp(buf, "C:\\RD\\CUR") == 0);
    CHECK(chdir("\\") == 0);
    CHECK(is_dir("RD\\CUR"));
    errno = 0;
    CHECK(rmdir("\\") == -1 && errno == EACCES);

    /* missing, a missing parent, a file and wildcards */
    errno = 0;
    CHECK(rmdir("RD\\NONE") == -1 && errno == ENOENT);
    errno = 0;
    CHECK(rmdir("RD\\NONE\\SUB") == -1 && errno == ENOENT);
    fd = open("RD\\F.TXT", O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    CHECK(fd >= 0 && close(fd) == 0);
    errno = 0;
    CHECK(rmdir("RD\\F.TXT") == -1 && (errno == ENOENT || errno == EACCES));
    CHECK(access("RD\\F.TXT", 0) == 0);
    CHECK(mkdir("RD\\W1") == 0);
    errno = 0;
    CHECK(rmdir("RD\\W*") == -1 && errno == ENOENT && is_dir("RD\\W1"));
    errno = 0;
    CHECK(rmdir("RD\\W?") == -1 && errno == ENOENT && is_dir("RD\\W1"));

    /* and all of it removed */
    CHECK(rmdir("RD\\W1") == 0);
    CHECK(unlink("RD\\F.TXT") == 0 && unlink("RD\\FULL\\F.TXT") == 0);
    CHECK(rmdir("RD\\FULL") == 0 && rmdir("RD\\CUR") == 0 && rmdir("RD") == 0);
    CHECK(chdir("RD") == -1);
    return done();
}
