/* spawnl, spawnv, spawnve, spawnvp, spawnvpe and getpid
 * (docs/spec/process.md). The cases set LIBC16=INHERIT and
 * PATH=C:\NOPE;;Q:\X;C:\BIN\;C:\BIN2 first. CHILD.EXE's output goes to
 * the same standard output, and process.expect checks it. */

#include <dos.h>
#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <string.h>
#include <sys/stat.h>
#include "check.h"

static unsigned block0, free0;
static char buf[512];

static void mark(const char *what)
{
    t_puts("-- ");
    t_puts(what);
    t_puts("\r\n");
}

/* The size in paragraphs of the program's memory block, from its MCB */
static unsigned block_size(void)
{
    return *(unsigned __far *)MK_FP(_psp - 1, 3);
}

/* The largest free block DOS has, in paragraphs */
static unsigned largest_free(void)
{
    union REGS r;

    r.h.ah = 0x48;
    r.x.bx = 0xffff;
    intdos(&r, &r);
    return r.x.bx;
}

/* The program's memory block and DOS's free memory are as at the start */
#define SAME_MEMORY() CHECK(block_size() == block0 && largest_free() == free0)

/* The call failed with -1 and err, and the memory is unchanged */
#define FAILS(call, err) do { errno = 0; CHECK((call) == -1); CHECK(errno == (err)); SAME_MEMORY(); } while (0)

/* The call returned code, left errno alone, and the memory is unchanged */
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

static char a125[126], a126[127], q123[124], q124[125];

/* Environments: 258 strings of 126 bytes make a 32767-byte block; one of
 * 127 bytes instead makes 32768 */
static char e126[127], e127[128];
static char *big[260];

int main(void)
{
    static char *codes0[] = { "child", "0", 0 };
    static char *codes7[] = { "child", "7", 0 };
    static char *codes255[] = { "child", "255", 0 };
    static char *quoting[] = { "child", "a b", "", "tab\there", "q\"uote", "back\\\\", "x\\\"y",
        "\\\\\"z", "end\\", "sp ace\\", "two  spaces", "1", 0 };
    static char *none[] = { "child", 0 };
    static char *fcb[] = { "child", "-f", "A:FOO.TXT", 0 };
    static char *three[] = { "child", "3", 0 };
    static char *kid[] = { "kid", "5", 0 };
    static char *showenv[] = { "child", "-e", 0 };
    static char *inherit[] = { "child", "-g", "LIBC16", 0 };
    static char *getpath[] = { "child", "-g", "PATH", 0 };
    static char *nullarg0[] = { 0 };
    static char *env[] = { "FOO=bar baz", "X=1", 0 };
    static char *env_a[] = { "A=1", 0 };
    static char *empty[] = { 0 };
    char *longarg[3];
    union REGS r;
    int i;

    block0 = block_size();
    free0 = largest_free();
    CHECK(free0 > 0x1000);

    mkdir("BIN");
    mkdir("BIN2");
    CHECK(copy("CHILD.EXE", "BIN\\KID.COM"));
    CHECK(copy("CHILD.EXE", "BIN\\KID.EXE"));
    CHECK(copy("CHILD.EXE", "BIN\\CHILD.EXE"));
    CHECK(copy("CHILD.EXE", "BIN2\\KID.COM"));
    CHECK(copy("CHILD.EXE", "BIN2\\KID2.EXE"));
    SAME_MEMORY();

    mark("exit codes");
    RUNS(spawnv(P_WAIT, "CHILD.EXE", codes0), 0);
    RUNS(spawnv(P_WAIT, "CHILD.EXE", codes7), 7);
    RUNS(spawnv(P_WAIT, "CHILD.EXE", codes255), 255);
    RUNS(spawnl(P_WAIT, "CHILD.EXE", "child", "l1", "l 2", "9", (char *)0), 9);
    RUNS(spawnl(P_WAIT, "CHILD.EXE", "child", (char *)0), 0);

    mark("quoting");
    RUNS(spawnv(P_WAIT, "CHILD.EXE", quoting), 1);
    RUNS(spawnv(P_WAIT, "CHILD.EXE", none), 0);

    mark("126 bytes");
    memset(a125, 'a', 125);
    memset(a126, 'a', 126);
    memset(q123, 'q', 123);
    q123[60] = ' ';
    memset(q124, 'q', 124);
    q124[60] = ' ';
    longarg[0] = "child";
    longarg[2] = 0;
    longarg[1] = a125;
    RUNS(spawnv(P_WAIT, "CHILD.EXE", longarg), 0);
    longarg[1] = q123;
    RUNS(spawnv(P_WAIT, "CHILD.EXE", longarg), 0);
    longarg[1] = a126;
    FAILS(spawnv(P_WAIT, "CHILD.EXE", longarg), E2BIG);
    longarg[1] = q124;
    FAILS(spawnv(P_WAIT, "CHILD.EXE", longarg), E2BIG);
    FAILS(spawnl(P_WAIT, "CHILD.EXE", "child", a125, "x", (char *)0), E2BIG);

    mark("fcb");
    RUNS(spawnv(P_WAIT, "CHILD.EXE", fcb), 0);

    mark("extensions");
    RUNS(spawnv(P_WAIT, "CHILD", three), 3);
    RUNS(spawnv(P_WAIT, "BIN\\KID", kid), 5);
    RUNS(spawnv(P_WAIT, "BIN/KID.EXE", kid), 5);
    FAILS(spawnv(P_WAIT, "KID", kid), ENOENT);
    FAILS(spawnv(P_WAIT, "KID.COM", kid), ENOENT);

    mark("path");
    RUNS(spawnvp(P_WAIT, "CHILD", three), 3);
    RUNS(spawnvp(P_WAIT, "KID", kid), 5);
    RUNS(spawnvp(P_WAIT, "KID.EXE", kid), 5);
    RUNS(spawnvp(P_WAIT, "KID2", kid), 5);
    RUNS(spawnvp(P_WAIT, "BIN2\\KID", kid), 5);
    FAILS(spawnvp(P_WAIT, ".\\KID", kid), ENOENT);
    FAILS(spawnvp(P_WAIT, "NOSUCH", kid), ENOENT);
    FAILS(spawnvp(P_WAIT, "KID.BIN", kid), ENOENT);
    RUNS(spawnvp(P_WAIT, "KID", getpath), 0);

    mark("environment");
    RUNS(spawnve(P_WAIT, "CHILD.EXE", showenv, env), 0);
    RUNS(spawnve(P_WAIT, "CHILD.EXE", showenv, empty), 0);
    RUNS(spawnve(P_WAIT, "CHILD.EXE", inherit, 0), 0);
    RUNS(spawnv(P_WAIT, "CHILD.EXE", inherit), 0);
    RUNS(spawnl(P_WAIT, "CHILD.EXE", "child", "-g", "LIBC16", (char *)0), 0);
    RUNS(spawnvp(P_WAIT, "KID", inherit), 0);
    RUNS(spawnvpe(P_WAIT, "KID", showenv, env_a), 0);
    RUNS(spawnvpe(P_WAIT, "CHILD", inherit, 0), 0);

    mark("32767 bytes");
    memset(e126, 'e', 126);
    e126[1] = '=';
    memset(e127, 'f', 127);
    e127[1] = '=';
    for (i = 0; i < 258; i++)
        big[i] = e126;
    big[258] = 0;
    RUNS(spawnve(P_WAIT, "CHILD.EXE", codes7, big), 7);
    big[100] = e127;
    FAILS(spawnve(P_WAIT, "CHILD.EXE", codes7, big), E2BIG);
    FAILS(spawnvpe(P_WAIT, "CHILD", codes7, big), E2BIG);

    mark("modes");
    FAILS(spawnv(P_NOWAIT, "CHILD.EXE", codes7), EINVAL);
    FAILS(spawnv(P_OVERLAY, "CHILD.EXE", codes7), EINVAL);
    FAILS(spawnv(99, "CHILD.EXE", codes7), EINVAL);
    FAILS(spawnve(P_NOWAIT, "CHILD.EXE", codes7, env), EINVAL);
    FAILS(spawnvp(P_OVERLAY, "CHILD", codes7), EINVAL);
    FAILS(spawnvpe(-1, "CHILD", codes7, env), EINVAL);
    FAILS(spawnl(P_NOWAIT, "CHILD.EXE", "child", "7", (char *)0), EINVAL);

    mark("bad arguments");
    FAILS(spawnv(P_WAIT, "CHILD.EXE", 0), EINVAL);
    FAILS(spawnv(P_WAIT, "CHILD.EXE", nullarg0), EINVAL);
    FAILS(spawnl(P_WAIT, "CHILD.EXE", (char *)0), EINVAL);
    FAILS(spawnve(P_WAIT, "CHILD.EXE", 0, env), EINVAL);
    FAILS(spawnvp(P_WAIT, "CHILD", 0), EINVAL);

    mark("missing");
    FAILS(spawnv(P_WAIT, "NOSUCH.EXE", codes7), ENOENT);
    FAILS(spawnv(P_WAIT, "NOSUCH", codes7), ENOENT);
    FAILS(spawnv(P_WAIT, "NODIR\\CHILD.EXE", codes7), ENOENT);
    FAILS(spawnve(P_WAIT, "NOSUCH", codes7, env), ENOENT);
    FAILS(spawnvpe(P_WAIT, "NOSUCH", codes7, env), ENOENT);

    mark("getpid");
    CHECK(getpid() == (int)_psp);
    r.h.ah = 0x62;
    intdos(&r, &r);
    CHECK(getpid() == (int)r.x.bx);

    unlink("BIN\\KID.COM");
    unlink("BIN\\KID.EXE");
    unlink("BIN\\CHILD.EXE");
    unlink("BIN2\\KID.COM");
    unlink("BIN2\\KID2.EXE");
    rmdir("BIN");
    rmdir("BIN2");
    return done();
}
