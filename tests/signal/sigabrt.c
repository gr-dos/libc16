/* abort with a SIGABRT disposition (docs/spec/signal.md):
 *   sigabrt ret   a handler that returns: called once, then exit code 3
 *   sigabrt ign   SIG_IGN: exit code 3
 *   sigabrt jump  a handler that longjmps out: the program goes on */

#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "testio.h"

static jmp_buf jb;
static int jump, calls;

static void __cdecl handler(int sig)
{
    void (__cdecl *p)(int) = signal(SIGABRT, SIG_DFL);

    calls++;
    t_puts(sig == SIGABRT && p == SIG_DFL && calls == 1 ? "handler\r\n" : "not ok handler\r\n");
    if (jump)
        longjmp(jb, 1);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;
    if (strcmp(argv[1], "ign") == 0)
        signal(SIGABRT, SIG_IGN);
    else {
        jump = strcmp(argv[1], "jump") == 0;
        signal(SIGABRT, handler);
    }
    if (setjmp(jb) != 0) {
        t_puts("back\r\n");
        return calls == 1 ? 0 : 1;
    }
    t_puts("aborting\r\n");
    abort();
    t_puts("not ok abort returned\r\n");
    return 1;
}
