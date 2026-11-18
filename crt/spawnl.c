/* spawnl - libc16 (docs/spec/process.md) */

#include <process.h>
#include "proc16.h"

/* The arguments after path are on the stack in order, a null-terminated
 * array of pointers like spawnv's argv. */
int __cdecl spawnl(int mode, const char *path, const char *arg0, ...)
{
    return __libc16_spawnx(mode, path, (char * const *)&arg0, NULL, __libc16_spawnext);
}
