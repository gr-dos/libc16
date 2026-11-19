/* A failed flush stops the child (docs/spec/process.md, "Across the
 * call"): with a buffered file stream whose DOS writes
 * tests/stdbuf/wrhook makes fail, or write short, spawnl, spawnve (with
 * an environment block to free), system and libc16_systeme return -1
 * with errno from the write, the stream's error indicator set, and run
 * no child; buffail.expect checks that no child printed anything then.
 * With the writes let through, each runs its child. Run with
 * COMSPEC=C:\CHILD.EXE. */

#include <dos.h>
#include <errno.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include "check.h"

void __cdecl wrhook_install(void);
void __cdecl wrhook_remove(void);
void __cdecl wrhook_watch(int fd);
void __cdecl wrhook_limit(unsigned skip, unsigned max);
void __cdecl wrhook_fail(unsigned skip, unsigned err);

static char buf[64];
static char *envp[] = { "A=1", NULL };
static char *argv[] = { "CHILD.EXE", NULL };

static void mark(const char *what)
{
    t_puts("-- ");
    t_puts(what);
    t_puts("\r\n");
}

static unsigned largest_free(void)
{
    union REGS r;

    r.h.ah = 0x48;
    r.x.bx = 0xFFFF;
    intdos(&r, &r);
    return r.x.bx;
}

/* Arms the hook on fp's handle with some output pending. */
static void pending(FILE *fp, int shortw)
{
    clearerr(fp);
    CHECK(fputs("pending", fp) == 0);
    if (shortw)
        wrhook_limit(0, 3);
    else
        wrhook_fail(0, 5);
    errno = 0;
}

static void after(FILE *fp, int r, int shortw)
{
    CHECK(r == -1 && errno == (shortw ? ENOSPC : EACCES) && ferror(fp));
    wrhook_watch(fileno(fp));
    CHECK(fflush(fp) == 0);
}

int main(void)
{
    FILE *fp = fopen("BUFFAIL.TXT", "w");
    unsigned free0;

    CHECK(fp != NULL && setvbuf(fp, buf, _IOFBF, sizeof buf) == 0);
    wrhook_install();
    wrhook_watch(fileno(fp));
    free0 = largest_free();

    mark("spawnl fails");
    pending(fp, 0);
    after(fp, spawnl(P_WAIT, "CHILD.EXE", "CHILD.EXE", "ran", NULL), 0);
    mark("spawnve fails short");
    pending(fp, 1);
    after(fp, spawnve(P_WAIT, "CHILD.EXE", argv, envp), 1);
    /* the environment block was freed */
    CHECK(largest_free() == free0);
    mark("system fails");
    pending(fp, 0);
    after(fp, system("ran"), 0);
    mark("libc16_systeme fails");
    pending(fp, 0);
    after(fp, libc16_systeme("ran", envp), 0);
    CHECK(largest_free() == free0);

    mark("all run");
    clearerr(fp);
    CHECK(fputs("written", fp) == 0);
    CHECK(spawnl(P_WAIT, "CHILD.EXE", "CHILD.EXE", "spawnl", NULL) == 0);
    CHECK(spawnve(P_WAIT, "CHILD.EXE", argv, envp) == 0);
    CHECK(system("system") == 0);
    CHECK(libc16_systeme("systeme", envp) == 0 && !ferror(fp));

    wrhook_remove();
    CHECK(fclose(fp) == 0);
    return done();
}
