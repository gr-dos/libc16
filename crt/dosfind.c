/* Directory searches for _dos_findfirst and _dos_findnext - libc16
 * (docs/spec/dos.md) */

#include "dos16.h"
#include "io16.h"

/* The disk transfer area's address, segment in DX and offset in AX */
static unsigned long get_dta(void);
#pragma aux get_dta = \
    "mov ah, 2fh" \
    "int 21h" \
    "mov dx, es" \
    "mov ax, bx" \
    value [dx ax] modify exact [ax bx dx es];

static void set_dta(void *dta);
#pragma aux set_dta = \
    "mov ah, 1ah" \
    "int 21h" \
    parm DS_DX modify exact [ax];

static void set_dta_far(unsigned seg, unsigned off);
#pragma aux set_dta_far = \
    "push ds" \
    "mov ds, cx" \
    "mov ah, 1ah" \
    "int 21h" \
    "pop ds" \
    parm [cx] [dx] modify exact [ax];

static unsigned long find(unsigned char fn, const char *path, unsigned attr);
#pragma aux find = \
    "int 21h" \
    "sbb dx, dx" \
    parm [ah] DS_DX [cx] value [dx ax] modify exact [ax dx];

unsigned __libc16_find(unsigned char fn, const char *path, unsigned attr, struct find_t *f)
{
    unsigned long dta = get_dta();
    unsigned long r;

    set_dta(f);
    r = find(fn, path, attr);
    set_dta_far((unsigned)(dta >> 16), (unsigned)dta);
    if (DOS_FAILED(r)) {
        __libc16_doserr(DOS_AX(r));
        return DOS_AX(r);
    }
    return 0;
}
