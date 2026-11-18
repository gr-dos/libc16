/* DOS restores INT 23h when a program that set a SIGINT handler ends
 * (docs/spec/signal.md): runs SIGKID.EXE and checks the vector. */

#include <dos.h>
#include <process.h>
#include "check.h"

int main(void)
{
    void (__interrupt __far *before)() = _dos_getvect(0x23);

    CHECK(spawnl(P_WAIT, "SIGKID.EXE", "sigkid", (char *)0) == 0);
    CHECK(_dos_getvect(0x23) == before);
    return done();
}
