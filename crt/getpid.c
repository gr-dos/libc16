/* getpid - libc16 (docs/spec/process.md) */

#include <dos.h>
#include <process.h>

int __cdecl getpid(void)
{
    return _psp;
}
