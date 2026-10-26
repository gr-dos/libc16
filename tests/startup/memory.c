/* Prints the size in paragraphs of the program's memory block (from its
 * MCB, the paragraph before the PSP), whether _psp matches INT 21h
 * function 62h, and the size of the largest free block. */

#include "testio.h"

extern unsigned _psp;

static unsigned dos_psp(void);
#pragma aux dos_psp = \
    "mov ah, 62h" \
    "int 21h" \
    value [bx] modify [ax bx];

static unsigned dos_largest(void);
#pragma aux dos_largest = \
    "mov ah, 48h" \
    "mov bx, 0FFFFh" \
    "int 21h" \
    value [bx] modify [ax bx];

int main(void)
{
    unsigned __far *mcb_size = ((__segment)(_psp - 1)) :> ((unsigned __near *)3);

    t_puts("block ");
    t_putu(*mcb_size);
    t_puts("\r\npsp ");
    t_puts(_psp == dos_psp() ? "ok" : "wrong");
    t_puts("\r\nfree ");
    t_putu(dos_largest());
    t_puts("\r\n");
    return 0;
}
