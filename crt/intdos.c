/* intdos - libc16 (docs/spec/dos.md) */

#include <dos.h>

int __cdecl intdos(union REGS *in, union REGS *out)
{
    return int86(0x21, in, out);
}
