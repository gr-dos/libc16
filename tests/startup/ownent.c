/* A program with its own entry point, tests/startup/ownent.asm, instead of
 * the core startup (docs/spec/startup.md, "Programs without the startup").
 * It defines no main, so cstart is not linked; it ends through exit. */

#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "testio.h"

void ownmain(void);

static char buf[16];            /* BSS, zeroed by the entry */
static int init = 3;            /* initialised data */

void ownmain(void)
{
    union REGS r;
    struct SREGS s;
    int ok = buf[0] == '\0' && buf[15] == '\0' && init == 3;

    strcpy(buf, "own");
    strcat(buf, " entry");
    ok = ok && strcmp(buf, "own entry") == 0 && strlen(buf) == 9;
    segread(&s);
    ok = ok && s.ds == s.ss;
    r.h.ah = 0x19;
    ok = ok && (intdos(&r, &r) & 0xff) == 2;
    t_puts(ok ? "ok\r\n" : "not ok\r\n");
    exit(ok ? 7 : 1);
}
