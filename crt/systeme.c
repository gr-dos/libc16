/* libc16_systeme - libc16 (docs/spec/process.md) */

#include <process.h>
#include "proc16.h"

int __cdecl libc16_systeme(const char *cmd, char * const *envp)
{
    return __libc16_system(cmd, envp, __libc16_envblock);
}
