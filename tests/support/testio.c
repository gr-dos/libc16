#include "testio.h"

static void dos_write(const char *buf, unsigned n);
#pragma aux dos_write = \
    "mov bx, 1" \
    "mov ah, 40h" \
    "int 21h" \
    parm T_DS_DX [cx] modify [ax bx];

void t_puts(const char *s)
{
    unsigned n = 0;

    while (s[n])
        n++;
    dos_write(s, n);
}

void t_putu(unsigned u)
{
    char buf[6];
    char *p = buf + sizeof buf - 1;

    *p = '\0';
    do {
        *--p = '0' + u % 10;
        u /= 10;
    } while (u);
    t_puts(p);
}
