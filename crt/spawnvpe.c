/* spawnvpe - libc16 (docs/spec/process.md) */

#include <process.h>
#include "proc16.h"

int __cdecl spawnvpe(int mode, const char *path, char * const *argv, char * const *envp)
{
    return __libc16_spawnx(mode, path, argv, envp, __libc16_spawnp);
}
