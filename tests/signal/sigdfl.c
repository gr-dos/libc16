/* raise with SIG_DFL (docs/spec/signal.md): ends the program with exit
 * code 3 without calling atexit functions. "sigdfl term" raises SIGTERM;
 * "sigdfl int" raises SIGINT after setting a handler and SIG_DFL again,
 * which must not reach INT 23h. */

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "testio.h"

static void __cdecl not_called(void)
{
    t_puts("not ok an atexit function ran\r\n");
}

static void __cdecl handler(int sig)
{
    (void)sig;
    t_puts("not ok the handler ran\r\n");
}

int main(int argc, char **argv)
{
    int sig = SIGTERM;

    if (atexit(not_called) != 0)
        t_puts("not ok atexit\r\n");
    if (argc == 2 && strcmp(argv[1], "int") == 0) {
        sig = SIGINT;
        if (signal(SIGINT, handler) != SIG_DFL || signal(SIGINT, SIG_DFL) != handler)
            t_puts("not ok signal\r\n");
    }
    t_puts("raising\r\n");
    raise(sig);
    t_puts("not ok raise returned\r\n");
    return 1;
}
