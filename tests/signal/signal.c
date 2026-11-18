/* signal and raise, and SIGINT through INT 23h (docs/spec/signal.md).
 * sigasm.asm issues INT 23h; DOS's own handler is never reached, since
 * the test first sets INT 23h to sg_prev, which signal saves. The output
 * is the ^C DOS writes when getch reads a Ctrl-C, then ok. */

#include <conio.h>
#include <dos.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include "check.h"

void __cdecl sg_set_prev(void);
unsigned __cdecl sg_int23(void);
void __cdecl sg_int23_ss(void);
unsigned __cdecl sg_flags(void);
extern unsigned __cdecl sg_prev_hits;

#define IF 0x200
#define DF 0x400

typedef void (__cdecl *handler)(int);

static unsigned dgroup;
static volatile int calls, last;
static unsigned h_flags, h_ds, h_ss;
static handler h_disp;
static jmp_buf jb;

/* The disposition of sig, unchanged */
static handler disposition(int sig)
{
    handler p = signal(sig, SIG_IGN);

    signal(sig, p);
    return p;
}

static void __cdecl record(int sig)
{
    struct SREGS s;

    h_flags = sg_flags();
    segread(&s);
    h_ds = s.ds;
    h_ss = s.ss;
    h_disp = disposition(sig);
    calls++;
    last = sig;
}

static void __cdecl again(int sig)
{
    calls++;
    last = sig;
    signal(sig, again);
}

static void __cdecl other(int sig)
{
    calls++;
    last = sig;
    CHECK(raise(SIGFPE) == 0);
}

static void __cdecl jumper(int sig)
{
    calls++;
    last = sig;
    longjmp(jb, sig);
}

/* The handler record saw DS and SS as DGROUP, interrupts on, the
 * direction flag clear and its disposition SIG_DFL */
static int recorded_int23(void)
{
    return h_ds == dgroup && h_ss == dgroup && (h_flags & IF) && !(h_flags & DF) && h_disp == SIG_DFL;
}

int main(void)
{
    struct SREGS s;
    union REGS r;
    int i, c;
    unsigned prev;

    segread(&s);
    dgroup = s.ss;
    sg_set_prev();

    /* every signal starts at SIG_DFL; signal returns the previous one */
    for (i = 1; i < NSIG; i++) {
        errno = 1234;
        CHECK(signal(i, SIG_IGN) == SIG_DFL);
        CHECK(signal(i, record) == SIG_IGN);
        CHECK(signal(i, SIG_DFL) == record);
        CHECK(signal(i, SIG_DFL) == SIG_DFL);
        CHECK(errno == 1234);
    }

    /* bad numbers and SIG_ERR */
    errno = 0;
    CHECK(signal(0, record) == SIG_ERR && errno == EINVAL);
    errno = 0;
    CHECK(signal(NSIG, record) == SIG_ERR && errno == EINVAL);
    errno = 0;
    CHECK(signal(-1, SIG_IGN) == SIG_ERR && errno == EINVAL);
    errno = 0;
    CHECK(signal(SIGTERM, SIG_ERR) == SIG_ERR && errno == EINVAL);
    CHECK(disposition(SIGTERM) == SIG_DFL);
    errno = 0;
    CHECK(raise(0) != 0 && errno == EINVAL);
    errno = 0;
    CHECK(raise(NSIG) != 0 && errno == EINVAL);

    /* raise with SIG_IGN */
    signal(SIGTERM, SIG_IGN);
    CHECK(raise(SIGTERM) == 0);
    CHECK(disposition(SIGTERM) == SIG_IGN);

    /* raise with a handler: reset to SIG_DFL first, called once */
    signal(SIGILL, record);
    calls = 0;
    errno = 1234;
    CHECK(raise(SIGILL) == 0);
    CHECK(errno == 1234);
    CHECK(calls == 1 && last == SIGILL && h_disp == SIG_DFL && h_ds == dgroup);
    CHECK(disposition(SIGILL) == SIG_DFL);

    /* a handler that sets itself again */
    signal(SIGSEGV, again);
    calls = 0;
    CHECK(raise(SIGSEGV) == 0 && raise(SIGSEGV) == 0);
    CHECK(calls == 2 && last == SIGSEGV && disposition(SIGSEGV) == again);
    signal(SIGSEGV, SIG_DFL);

    /* a handler that raises another signal */
    signal(SIGTERM, other);
    signal(SIGFPE, record);
    calls = 0;
    CHECK(raise(SIGTERM) == 0);
    CHECK(calls == 2 && last == SIGFPE);
    CHECK(disposition(SIGTERM) == SIG_DFL && disposition(SIGFPE) == SIG_DFL);

    /* a handler that jumps out, for a number with no name */
    signal(30, jumper);
    calls = 0;
    if (setjmp(jb) == 0) {
        raise(30);
        CHECK(0);
    } else
        CHECK(calls == 1 && last == 30 && disposition(30) == SIG_DFL);

    /* INT 23h with a handler */
    CHECK(signal(SIGINT, record) == SIG_DFL);
    calls = 0;
    prev = sg_prev_hits;
    CHECK(sg_int23() == 0);
    CHECK(calls == 1 && last == SIGINT && recorded_int23());
    CHECK(sg_prev_hits == prev);

    /* now SIG_DFL: the vector signal saved, sg_prev */
    CHECK(sg_int23() == 0);
    CHECK(calls == 1 && sg_prev_hits == prev + 1);
    CHECK(disposition(SIGINT) == SIG_DFL);

    /* SIG_IGN */
    CHECK(signal(SIGINT, SIG_IGN) == SIG_DFL);
    CHECK(sg_int23() == 0);
    CHECK(calls == 1 && sg_prev_hits == prev + 1);
    CHECK(disposition(SIGINT) == SIG_IGN);

    /* a handler that jumps out of INT 23h; interrupts stay enabled */
    signal(SIGINT, jumper);
    calls = 0;
    if (setjmp(jb) == 0) {
        sg_int23();
        CHECK(0);
    } else
        CHECK(calls == 1 && last == SIGINT && (sg_flags() & IF));
    CHECK(disposition(SIGINT) == SIG_DFL);

    /* SS not DGROUP: the saved vector, not the handler */
    signal(SIGINT, record);
    calls = 0;
    prev = sg_prev_hits;
    sg_int23_ss();
    CHECK(calls == 0 && sg_prev_hits == prev + 1);
    CHECK(disposition(SIGINT) == record);
    signal(SIGINT, SIG_IGN);
    sg_int23_ss();
    CHECK(calls == 0 && sg_prev_hits == prev + 1);

    /* a Ctrl-C read by DOS (function 08h) */
    signal(SIGINT, record);
    calls = 0;
    r.h.ah = 5;
    r.x.cx = 0x2e03;
    int86(0x16, &r, &r);
    CHECK(r.h.al == 0);
    r.h.ah = 5;
    r.x.cx = 0x2c7a;
    int86(0x16, &r, &r);
    c = getch();
    CHECK(c == 3 || c == 'z');
    while (kbhit())
        getch();
    CHECK(calls == 1 && last == SIGINT && recorded_int23());
    CHECK(sg_prev_hits == prev + 1);

    return done();
}
