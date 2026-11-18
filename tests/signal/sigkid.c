/* Run by sigvec (docs/spec/signal.md): sets a SIGINT handler, which
 * changes INT 23h, and exits; exit code 0 if it did. */

#include <dos.h>
#include <signal.h>

static void __cdecl handler(int sig)
{
    (void)sig;
}

int main(void)
{
    void (__interrupt __far *before)() = _dos_getvect(0x23);

    if (signal(SIGINT, handler) != SIG_DFL)
        return 1;
    return _dos_getvect(0x23) == before ? 2 : 0;
}
