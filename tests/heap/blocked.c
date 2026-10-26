/* malloc when DOS cannot grow the program's memory block: allocating
 * the largest free DOS block takes the memory above the program, so
 * INT 21h function 4Ah fails and malloc must return NULL with ENOMEM.
 * Once that block is freed, malloc works again. */

#include <errno.h>
#include <stdlib.h>
#include "testio.h"

static unsigned dos_largest(void);
#pragma aux dos_largest = \
    "mov ah, 48h" \
    "mov bx, 0FFFFh" \
    "int 21h" \
    value [bx] modify [ax bx];

/* The new block's segment, or 0. */
static unsigned dos_alloc(unsigned paras);
#pragma aux dos_alloc = \
    "mov ah, 48h" \
    "int 21h" \
    "sbb bx, bx" \
    "not bx" \
    "and ax, bx" \
    parm [bx] value [ax] modify [ax bx];

static void dos_free(unsigned seg);
#pragma aux dos_free = \
    "mov ah, 49h" \
    "int 21h" \
    parm [es] modify [ax];

static int failed;

static void check(const char *what, int ok)
{
    t_puts(what);
    t_puts(ok ? " ok\r\n" : " FAILED\r\n");
    if (!ok)
        failed = 1;
}

int main(void)
{
    unsigned seg = dos_alloc(dos_largest());
    void *p;

    check("DOS block allocated", seg != 0);
    errno = 0;
    p = malloc(1000);
    check("malloc fails with ENOMEM", p == NULL && errno == ENOMEM);
    dos_free(seg);
    p = malloc(1000);
    check("malloc works once DOS memory is free", p != NULL);
    free(p);
    return failed;
}
