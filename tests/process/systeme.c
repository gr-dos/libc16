/* libc16_systeme (docs/spec/process.md). The cases run it once for each
 * COMSPEC, as system.c:
 *   systeme fake  COMSPEC=C:\CHILD.EXE: the tail and environment the
 *                 command processor gets, the exit code, the limits
 *   systeme none  COMSPEC unset, even when envp sets it
 *   systeme dir   COMSPEC=C:\SYSDIR, a directory the test makes
 *   systeme real  COMSPEC=Z:\COMMAND.COM, DOSBox-X's command processor
 * CHILD.EXE's output goes to the same standard output, and the .expect
 * files check it. CHILD.EXE prints its command tail byte for byte when
 * TAIL is in its environment. */

#include <dos.h>
#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
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

/* Takes DOS's largest free block; its segment, or 0 */
static unsigned take_all(void)
{
    union REGS r;

    r.h.ah = 0x48;
    r.x.bx = largest_free();
    intdos(&r, &r);
    return r.x.cflag ? 0 : r.x.ax;
}

static void give_back(unsigned seg)
{
    union REGS r;
    struct SREGS s;

    segread(&s);
    s.es = seg;
    r.h.ah = 0x49;
    intdosx(&r, &r, &s);
}

#define SAME_MEMORY() CHECK(block_size() == block0 && largest_free() == free0)
#define FAILS(call, err) do { errno = 0; CHECK((call) == -1); CHECK(errno == (err)); SAME_MEMORY(); } while (0)
#define RUNS(call, code) do { errno = 1234; CHECK((call) == (code)); CHECK(errno == 1234); SAME_MEMORY(); } while (0)

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

/* A command with what spawnve would quote or escape, and what a command
 * processor gives meaning to, for the tail to keep byte for byte. */
static const char raw[] = "say \"a \\\"b\\\"\" c\\d\\\\ \"\" e\t%P% <i >o |m; 3";

static char *tail_env[] = { "TAIL=1", "COMSPEC=C:\\NOPE.EXE", 0 };
static char *changed[] = { "LIBC16=NEW", "ADDED=yes", 0 };
static char *deleted[] = { "ADDED=yes", 0 };
static char *empty[] = { 0 };

static char *too_big[3];

static void fake(void)
{
    char *big;

    mark("fake");
    CHECK(libc16_systeme(NULL, empty) == 1);
    CHECK(libc16_systeme(NULL, NULL) == 1);

    /* The tail is " /C " and the command, byte for byte; COMSPEC is the
     * parent's, though envp names another; the child sees envp. */
    RUNS(libc16_systeme(raw, tail_env), 3);
    RUNS(libc16_systeme("-e 6", tail_env), 6);

    /* The environment: changed and added, deleted, empty, inherited */
    RUNS(libc16_systeme("-g LIBC16", changed), 0);
    RUNS(libc16_systeme("-g ADDED", changed), 0);
    RUNS(libc16_systeme("-g LIBC16", deleted), 0);
    RUNS(libc16_systeme("-e 1", empty), 1);
    RUNS(libc16_systeme("-g LIBC16 2", NULL), 2);
    RUNS(libc16_systeme("-g ADDED", NULL), 0);
    RUNS(libc16_systeme("", empty), 0);

    /* 122 bytes of command run; 123 do not, with or without envp */
    memset(cmd, 'y', 120);
    strcpy(cmd + 120, " 4");
    RUNS(libc16_systeme(cmd, empty), 4);
    memset(cmd, 'y', 121);
    strcpy(cmd + 121, " 4");
    FAILS(libc16_systeme(cmd, empty), E2BIG);
    FAILS(libc16_systeme(cmd, NULL), E2BIG);

    /* An environment of more than 32767 bytes */
    big = malloc(20000);
    CHECK(big != NULL);
    if (big) {
        memset(big, 'v', 19999);
        big[0] = 'V';
        big[1] = '=';
        big[19999] = 0;
        too_big[0] = big;
        too_big[1] = big;
        block0 = block_size();      /* the heap grew the program's block */
        free0 = largest_free();
        FAILS(libc16_systeme("x 5", too_big), E2BIG);
        free(big);
    }
}

/* Without memory for the environment block: ENOMEM, nothing run */
static void nomem(void)
{
    unsigned seg;

    mark("nomem");
    seg = take_all();
    CHECK(seg != 0);
    errno = 0;
    CHECK(libc16_systeme("x 5", empty) == -1);
    CHECK(errno == ENOMEM);
    give_back(seg);
    SAME_MEMORY();
}

static void none(void)
{
    static char *comspec[] = { "COMSPEC=C:\\CHILD.EXE", 0 };

    mark("none");
    CHECK(libc16_systeme(NULL, comspec) == 0);
    FAILS(libc16_systeme("x 9", comspec), ENOENT);
    FAILS(libc16_systeme("x 9", NULL), ENOENT);
}

static void dir(void)
{
    mark("dir");
    CHECK(mkdir("SYSDIR") == 0);
    CHECK(libc16_systeme(NULL, empty) == 0);
    errno = 0;
    CHECK(libc16_systeme("x 9", empty) == -1);
    CHECK(errno != 0);
    CHECK(rmdir("SYSDIR") == 0);
}

static void real(void)
{
    static char *env[] = { "COMSPEC=Z:\\COMMAND.COM", "PATH=Z:\\", "GIVEN=\"q\" r", 0 };
    int r;

    mark("real");
    CHECK(libc16_systeme(NULL, env) == 1);
    unlink("SYSE.TXT");
    errno = 1234;
    r = libc16_systeme("CHILD \"a b\" -g GIVEN >SYSE.TXT", env);
    CHECK(r != -1);
    CHECK(errno == 1234);
    SAME_MEMORY();
    CHECK(file_has("SYSE.TXT", "arg[a b]"));
    CHECK(file_has("SYSE.TXT", "name[C:\\CHILD.EXE]"));
    unlink("SYSE.TXT");
    r = libc16_systeme("CHILD -g GIVEN >SYSE.TXT", env);
    CHECK(r != -1);
    CHECK(file_has("SYSE.TXT", "val[\"q\" r]"));
    unlink("SYSE.TXT");
}

int main(int argc, char **argv)
{
    block0 = block_size();
    free0 = largest_free();
    if (argc != 2)
        CHECK(0);
    else if (strcmp(argv[1], "fake") == 0) {
        fake();
        nomem();
    } else if (strcmp(argv[1], "none") == 0)
        none();
    else if (strcmp(argv[1], "dir") == 0)
        dir();
    else if (strcmp(argv[1], "real") == 0)
        real();
    else
        CHECK(0);
    return done();
}
