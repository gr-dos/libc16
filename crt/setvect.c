/* _dos_setvect - libc16 (docs/spec/dos.md) */

#include <dos.h>

static void set_vect(unsigned char intno, void (__interrupt __far *handler)());
#pragma aux set_vect = \
    "push ds" \
    "mov ds, cx" \
    "mov ah, 25h" \
    "int 21h" \
    "pop ds" \
    parm [al] [cx dx] modify exact [ax];

void __cdecl _dos_setvect(unsigned intno, void (__interrupt __far *handler)())
{
    set_vect((unsigned char)intno, handler);
}
