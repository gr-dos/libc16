/* malloc - libc16 (docs/spec/heap.md)
 *
 * Segregated free lists with boundary tags (crt/heap16.h). malloc rounds
 * the request up to a block size, takes the first free block that holds
 * it from the list of its size class, unless that list is known to hold
 * none, or else the first block of the next larger class that has one;
 * it splits off what it does not need and frees that. When no free block
 * will do, it grows the heap at its end, from a free block there if
 * there is one, growing the program's DOS memory block (INT 21h function
 * 4Ah); with far data it starts a new arena when the last one cannot
 * hold the block. Each call passes no block that a previous search found
 * too small, and the classes are a fixed few, so its time does not grow
 * with the number of blocks (the spec's Performance). */

#include <errno.h>
#include <stdlib.h>
#include <dos.h>
#include "heap16.h"

extern char __cdecl __libc16_stack_top[];
extern unsigned _psp;

unsigned *__libc16_heap_end;    /* null until the first block */

/* SS is DGROUP in every model (docs/ABI.md, "Memory models"). */
static unsigned dgroup(void);
#pragma aux dgroup = \
    "mov ax, ss" \
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

#ifdef HEAP_FAR
#define SEG(b) FP_SEG(b)
#define OFF(b) FP_OFF(b)
#else
#define SEG(b) dgroup()
#define OFF(b) ((unsigned)(b))
#endif

int __libc16_heap_grow(unsigned *b, unsigned need)
{
    unsigned seg = SEG(b), off = OFF(b), below, paras;

    if (need > ARENA_END - off)
        return 0;
    off += need;
    /* The memory block reaches past the new sentinel; a size that would
     * not fit in a word fails instead of wrapping to a smaller block. */
    below = seg - _psp;
    paras = below + ((off + 1) >> 4) + 1;
    if (paras <= below || !set_block(_psp, paras))
        return 0;
    b = AT(b, need);
    *b = USED;
    __libc16_heap_end = b;
    return 1;
}

void * __cdecl malloc(size_t size)
{
    unsigned need, have, c, *b, *last = 0;
    struct hfree *f;

    if (size > HEAP_MAX)
        goto fail;
    need = (size + 5) & ~3u;    /* the header, rounded up to 4 */
    if (need < MIN_BLOCK)
        need = MIN_BLOCK;
    c = __libc16_heap_class(need);
    if (need < ~__libc16_heap_fit[c]) {
        for (f = __libc16_heap_list[c]; f; f = f->next)
            if (f->head >= need)
                goto found;
        __libc16_heap_fit[c] = ~need;
    }
    /* The next class whose bit is set in the map, clearing the bits of
     * lists found empty. */
    for (c++; c < HEAP_CLASSES; c++) {
        have = __libc16_heap_map[c >> 4] >> (c & 15);
        if (!have) {
            c |= 15;
            continue;
        }
        for (; !(have & 1); have >>= 1)
            c++;
        if ((f = __libc16_heap_list[c]) != 0)
            goto found;
        __libc16_heap_map[c >> 4] &= ~(1u << (c & 15));
    }

    /* Grow the heap at its end, from the free block there if any. */
    b = __libc16_heap_end;
    if (b) {
        if (*b & PFREE)
            b = last = AT(b, -b[-1]);
    } else {
#ifdef HEAP_FAR
        b = MK_FP(dgroup() + (((unsigned)__libc16_stack_top - 1) >> 4) + 1, 0);
#else
        /* An odd stack top is rounded up; one at the end of a 64 KiB
         * DGROUP leaves no room. */
        b = (unsigned *)(((unsigned)__libc16_stack_top + 1) & ~1u);
        if (!b)
            goto fail;
#endif
    }
    if (!__libc16_heap_grow(b, need)) {
#ifdef HEAP_FAR
        /* A new arena at the paragraph after the last. Its sentinel moves
         * up to the word before that paragraph's last word, freeing the
         * bytes it passes. */
        b = __libc16_heap_end;
        if (!b)
            goto fail;
        c = FP_SEG(b) + (FP_OFF(b) >> 4) + 1;
        if (c < FP_SEG(b) || !__libc16_heap_grow(MK_FP(c, 0), need))
            goto fail;
        have = 12 - (FP_OFF(b) & 15);
        if (have) {
            *AT(b, have) = USED;
            *b = (*b & PFREE) | have | USED;
            free(b + 1);
        }
        last = 0;
        b = MK_FP(c, 0);
#else
        goto fail;
#endif
    }
    if (last && *last >= MIN_BLOCK)
        __libc16_heap_unlink((struct hfree *)last);
    *b = need | USED;
    return b + 1;

found:
    __libc16_heap_unlink(f);
    b = &f->head;
    have = *b;
    *b = need | USED;
    if (have != need) {
        /* The rest, after this block and before an allocated one */
        b = AT(f, need);
        *b = (have - need) | USED;
        free(b + 1);
    } else
        *AT(f, have) &= ~PFREE;
    return &f->head + 1;

fail:
    errno = ENOMEM;
    return NULL;
}
