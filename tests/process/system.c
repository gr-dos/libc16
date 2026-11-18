/* system (docs/spec/process.md). The cases run it once for each COMSPEC:
 *   system fake  COMSPEC=C:\CHILD.EXE: the exit code passes through
 *   system none  COMSPEC unset: \COMMAND.COM, which the test makes
 *   system dir   COMSPEC=C:\SYSDIR, a directory the test makes
 *   system real  COMSPEC=Z:\COMMAND.COM, DOSBox-X's command processor
 * CHILD.EXE's output goes to the same standard output, and the .expect
 * files check it. */

#include <dos.h>
#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "check.h"

static unsigned block0, free0;
static char buf[512];
static char cmd[130];

static void mark(const char *what)
{
    t_puts("-- ");
    t_puts(what);
    t_puts("\r\n");
}

static unsigned block_size(void)
{
    return *(unsigned __far *)MK_FP(_psp - 1, 3);
}

static unsigned largest_free(void)
{
    union REGS r;

    r.h.ah = 0x48;
    r.x.bx = 0xffff;
    intdos(&r, &r);
    return r.x.bx;
}

#define SAME_MEMORY() CHECK(block_size() == block0 && largest_free() == free0)
#define FAILS(call, err) do { errno = 0; CHECK((call) == -1); CHECK(errno == (err)); SAME_MEMORY(); } while (0)
#define RUNS(call, code) do { errno = 1234; CHECK((call) == (code)); CHECK(errno == 1234); SAME_MEMORY(); } while (0)

static int copy(const char *from, const char *to)
{
    int in = open(from, O_RDONLY | O_BINARY);
    int out = open(to, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
    int n = 0, ok = in >= 0 && out >= 0;

    while (ok && (n = read(in, buf, sizeof buf)) > 0)
        ok = write(out, buf, n) == n;
    if (n < 0)
        ok = 0;
    if (in >= 0)
        close(in);
    if (out >= 0 && close(out) != 0)
        ok = 0;
    return ok;
}

/* The file's text contains want */
static int file_has(const char *path, const char *want)
{
    int fd = open(path, O_RDONLY | O_BINARY);
    int n = fd < 0 ? -1 : read(fd, buf, sizeof buf - 1);

    if (fd >= 0)
        close(fd);
    if (n < 0)
        return 0;
    buf[n] = 0;
    return strstr(buf, want) != NULL;
}

static void fake(void)
{
    mark("fake");
    CHECK(system(NULL) == 1);
    RUNS(system("x 9"), 9);
    RUNS(system("\"a b\"  0"), 0);
    RUNS(system(""), 0);

    /* 122 bytes of command run; 123 do not */
    memset(cmd, 'y', 120);
    strcpy(cmd + 120, " 4");
    RUNS(system(cmd), 4);
    memset(cmd, 'y', 121);
    strcpy(cmd + 121, " 4");
    FAILS(system(cmd), E2BIG);
}

static void none(void)
{
    mark("none");
    CHECK(system(NULL) == 0);
    FAILS(system("x 9"), ENOENT);
    CHECK(copy("CHILD.EXE", "\\COMMAND.COM"));
    CHECK(system(NULL) == 1);
    RUNS(system("y 4"), 4);
    CHECK(unlink("\\COMMAND.COM") == 0);
    CHECK(system(NULL) == 0);
}

static void dir(void)
{
    mark("dir");
    CHECK(mkdir("SYSDIR") == 0);
    CHECK(system(NULL) == 0);
    errno = 0;
    CHECK(system("x 9") == -1);
    CHECK(errno != 0);
    CHECK(rmdir("SYSDIR") == 0);
    CHECK(system(NULL) == 0);
}

static void real(void)
{
    int r;

    mark("real");
    CHECK(system(NULL) == 1);
    unlink("SYS.TXT");
    errno = 1234;
    r = system("CHILD 5 >SYS.TXT");
    CHECK(r != -1);
    CHECK(errno == 1234);
    SAME_MEMORY();
    CHECK(file_has("SYS.TXT", "arg[5]"));
    CHECK(file_has("SYS.TXT", "name[C:\\CHILD.EXE]"));
    unlink("SYS.TXT");
}

int main(int argc, char **argv)
{
    block0 = block_size();
    free0 = largest_free();
    if (argc != 2)
        CHECK(0);
    else if (strcmp(argv[1], "fake") == 0)
        fake();
    else if (strcmp(argv[1], "none") == 0)
        none();
    else if (strcmp(argv[1], "dir") == 0)
        dir();
    else if (strcmp(argv[1], "real") == 0)
        real();
    else
        CHECK(0);
    return done();
}
