/* envblock - libc16 (docs/spec/process.md): the environment block of
 * spawnve, spawnvpe and libc16_systeme */

#include <errno.h>
#include "io16.h"
#include "proc16.h"

/* INT 21h 48h: the segment, or DX 0xFFFF on failure */
static unsigned long dos_alloc(unsigned paras);
#pragma aux dos_alloc = \
    "mov ah, 48h" \
    "int 21h" \
    "sbb dx, dx" \
    parm [bx] value [dx ax] modify exact [ax bx dx];

unsigned __libc16_envblock(char * const *envp)
{
    char * const *v;
    const char *s;
    char __far *e;
    unsigned long a;
    unsigned n, seg;

    n = 1;
    for (v = envp; (s = *v) != NULL; v++) {
        do {
            if (++n > 0x7fff) {
                errno = E2BIG;
                return 0;
            }
        } while (*s++ != '\0');
    }
    /* one byte more, for an empty block's second zero */
    a = dos_alloc((n >> 4) + 1);
    if (DOS_FAILED(a)) {
        _doserrno = DOS_AX(a);
        errno = ENOMEM;
        return 0;
    }
    seg = DOS_AX(a);
    e = ((__segment)seg) :> ((char __near *)0);
    e[1] = '\0';
    for (v = envp; (s = *v) != NULL; v++)
        while ((*e++ = *s++) != '\0')
            ;
    *e = '\0';
    return seg;
}
