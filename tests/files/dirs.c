/* getcwd and chdir (docs/spec/files.md) */

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

/* getcwd fills buf with want */
static int cwd_is(const char *want)
{
    memset(buf, 0x55, sizeof buf);
    return getcwd(buf, sizeof buf) == buf && strcmp(buf, want) == 0;
}

static void cleanup(void)
{
    del_file("CWD\\F.TXT");
    del_dir("CWD\\SUB");
    del_dir("CWD");
}

int main(void)
{
    int fd;

    cleanup();

    /* the root, and buffer sizes */
    CHECK(cwd_is("C:\\"));
    CHECK(getcwd(buf, 4) == buf && strcmp(buf, "C:\\") == 0);
    errno = 0;
    CHECK(getcwd(buf, 3) == NULL && errno == ERANGE);
    errno = 0;
    CHECK(getcwd(buf, 0) == NULL && errno == ERANGE);
    errno = 0;
    CHECK(getcwd(buf, -1) == NULL && errno == ERANGE);
    errno = 0;
    CHECK(getcwd(NULL, 80) == NULL && errno == EINVAL);

    CHECK(mkdir("CWD") == 0 && mkdir("CWD\\SUB") == 0);
    fd = open("CWD\\F.TXT", O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    CHECK(fd >= 0 && close(fd) == 0);

    /* relative, .., the root and full paths */
    CHECK(chdir("CWD") == 0 && cwd_is("C:\\CWD"));
    CHECK(chdir("SUB") == 0 && cwd_is("C:\\CWD\\SUB"));
    CHECK(getcwd(buf, 11) == buf && strcmp(buf, "C:\\CWD\\SUB") == 0);
    errno = 0;
    CHECK(getcwd(buf, 10) == NULL && errno == ERANGE);
    CHECK(chdir("..") == 0 && cwd_is("C:\\CWD"));
    CHECK(chdir("\\") == 0 && cwd_is("C:\\"));
    CHECK(chdir("C:\\CWD\\SUB") == 0 && cwd_is("C:\\CWD\\SUB"));
    CHECK(chdir("c:..") == 0 && cwd_is("C:\\CWD"));

    /* missing directories and a file: nothing changes */
    errno = 0;
    CHECK(chdir("NOPE") == -1 && errno == ENOENT && cwd_is("C:\\CWD"));
    errno = 0;
    CHECK(chdir("F.TXT") == -1 && errno == ENOENT && cwd_is("C:\\CWD"));
    errno = 0;
    CHECK(chdir("\\CWD\\NOPE\\SUB") == -1 && errno == ENOENT && cwd_is("C:\\CWD"));

    /* another drive's directory: the current drive stays C: */
    CHECK(chdir("Z:\\") == 0 && cwd_is("C:\\CWD"));
    errno = 0;
    CHECK(chdir("Z:\\NOPE") == -1 && errno == ENOENT && cwd_is("C:\\CWD"));

    CHECK(chdir("\\") == 0 && cwd_is("C:\\"));
    cleanup();
    return done();
}
