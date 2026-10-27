/* intdosx - libc16 (docs/spec/dos.md) */

#include <dos.h>

int __cdecl intdosx(union REGS *in, union REGS *out, struct SREGS *segs)
{
    return int86x(0x21, in, out, segs);
}
