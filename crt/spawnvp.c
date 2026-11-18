/* spawnvp - libc16 (docs/spec/process.md) */

#include <process.h>
#include "proc16.h"

int __cdecl spawnvp(int mode, const char *path, char * const *argv)
{
    return __libc16_spawnx(mode, path, argv, NULL, __libc16_spawnp);
}
