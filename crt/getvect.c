/* _dos_getvect - libc16 (docs/spec/dos.md) */

#include <dos.h>

static void (__interrupt __far *get_vect(unsigned char intno))();
#pragma aux get_vect = \
    "mov ah, 35h" \
    "int 21h" \
    "mov dx, es" \
    "mov ax, bx" \
    parm [al] value [dx ax] modify exact [ax bx dx es];

void (__interrupt __far * __cdecl _dos_getvect(unsigned intno))()
{
    return get_vect((unsigned char)intno);
}
