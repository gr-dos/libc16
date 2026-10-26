/* malloc - libc16 (docs/spec/heap.md)
 *
 * The heap is a run of blocks in DGROUP from the top of the stack
 * (__libc16_stack_top, crt/cstart.asm) up to heap_end. Each block starts
 * with a header word: the block's size in bytes, header included, which
 * is even, with bit 0 set while the block is allocated. free (crt/free.c)
 * clears that bit. malloc takes the first free block that is large enough,
 * merging each free block with the free blocks after it as it goes, and
 * splits off what it does not need. When no block will do, it grows the
 * program's DOS memory block (INT 21h function 4Ah) to make room at
 * heap_end, reusing a free block that ends there. */

#include <errno.h>
#include <stdlib.h>

extern char __cdecl __libc16_stack_top[];
extern unsigned _psp;

static unsigned *heap_start;    /* null until the first call */
static unsigned *heap_end;

static unsigned dgroup(void);
#pragma aux dgroup = \
    "mov ax, ds" \
    value [ax];

/* Resizes the block at segment psp to paras paragraphs; nonzero if DOS
 * did. */
static int set_block(unsigned psp, unsigned paras);
#pragma aux set_block = \
    "mov ah, 4Ah" \
    "int 21h" \
    "sbb ax, ax" \
    "inc ax" \
    parm [es] [bx] value [ax] modify [ax bx];

#define NEXT(p, size) ((unsigned *)((char *)(p) + (size)))

void * __cdecl malloc(size_t size)
{
    unsigned *p, *next;
    unsigned *last = NULL;      /* a free block ending at heap_end */
    unsigned need = size + 3;   /* the header, rounded up to even */
    unsigned have, top;

    if (need < size)
        goto fail;
    need &= ~1u;
    if (!heap_start) {
        /* An odd stack top is rounded up; one at the end of a 64 KiB
         * DGROUP leaves no room. */
        top = ((unsigned)__libc16_stack_top + 1) & ~1u;
        if (!top)
            goto fail;
        heap_start = heap_end = (unsigned *)top;
    }
    for (p = heap_start; p != heap_end; p = NEXT(p, have & ~1u)) {
        have = *p;
        if (have & 1)
            continue;
        for (;;) {
            next = NEXT(p, have);
            if (next == heap_end || (*next & 1))
                break;
            have += *next;
        }
        if (have >= need) {
            if (have != need)
                *NEXT(p, need) = have - need;
            *p = need | 1;
            return p + 1;
        }
        *p = have;
        if (next == heap_end)
            last = p;
    }

    /* Grow, keeping DGROUP below 64 KiB: the block is DGROUP's
     * paragraphs above the PSP plus those up to the new top. */
    if (last)
        p = last;
    top = (unsigned)p + need;
    if (top < (unsigned)p || !set_block(_psp, dgroup() - _psp + ((top - 1) >> 4) + 1))
        goto fail;
    *p = need | 1;
    heap_end = (unsigned *)top;
    return p + 1;

fail:
    errno = ENOMEM;
    return NULL;
}
