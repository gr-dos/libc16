/* system - libc16 (docs/spec/process.md); and the core it shares with
 * libc16_systeme */

#include <errno.h>
#include <stdlib.h>
#include "io16.h"
#include "proc16.h"
#include "stdio16.h"

int __libc16_system(const char *cmd, char * const *envp, __libc16_envfn env)
{
    char tail[TAIL_MAX + 2];
    const char *sh;
    char *d;
    unsigned long a;
    unsigned seg;
    int r;

    if ((sh = getenv("COMSPEC")) == NULL)
        sh = "\\COMMAND.COM";
    /* It exists and is not a directory. DOSBox-X runs its own command
     * processor for 4B00h of any missing ...COMMAND.COM, so a missing one
     * is found here rather than by 4B00h. */
    a = __libc16_dos_getattr(sh);
    if (cmd == NULL)
        return !DOS_FAILED(a) && (DOS_AX(a) & 0x10) == 0;
    if (DOS_FAILED(a))
        return __libc16_doserr(DOS_AX(a));
    d = tail + 1;
    d[0] = ' ';
    d[1] = '/';
    d[2] = 'C';
    d[3] = ' ';
    for (d += 4; (*d = *cmd) != '\0'; d++, cmd++)
        if (d == tail + 1 + TAIL_MAX) {
            errno = E2BIG;
            return -1;
        }
    *d = '\r';
    tail[0] = (char)(d - tail - 1);
    seg = 0;
    if (env != NULL && envp != NULL && (seg = env(envp)) == 0)
        return -1;
    /* Buffered output before the child's, which shares the handles; a
     * child must not run without it. */
    if (FLUSH_FAILED(seg))
        return -1;
    r = __libc16_dosexec(sh, seg, tail);
    if (seg != 0)
        __libc16_dosfree(seg);
    if (r < 0)
        return __libc16_doserr(-r);
    return r;
}

int __cdecl system(const char *cmd)
{
    return __libc16_system(cmd, NULL, NULL);
}
