/* malloc - libc16 (docs/spec/heap.md)
 *
 * Small model: the heap is a run of blocks in DGROUP from the top of the
 * stack (__libc16_stack_top, crt/cstart.asm) up to heap_end. Each block
 * starts with a header word: the block's size in bytes, header included,
 * which is even, with bit 0 set while the block is allocated. free
 * (crt/free.c) clears that bit. malloc takes the first free block that is
 * large enough, merging each free block with the free blocks after it as
 * it goes, and splits off what it does not need. When no block will do,
 * it grows the program's DOS memory block (INT 21h function 4Ah) to make
 * room at heap_end, reusing a free block that ends there.
 *
 * Compact and large models: the heap is a run of arenas, each inside one
 * segment, from the paragraph after the top of the stack up through the
 * program's DOS memory block (crt/heap16.h). Blocks within an arena are
 * as in the small model, and found, merged and split the same way, but
 * the search starts at the first block that may be free
 * (__libc16_heap_seg:__libc16_heap_off) rather than at the first arena,
 * so that a program that only allocates (an editor reading its lines)
 * spends no time passing the blocks it allocated before (the spec's
 * Constraints). When no block will do, malloc grows the last arena, from
 * a free block at its end if there is one, and when that would take the
 * arena past ARENA_END it starts a new arena at the paragraph after it,
 * growing the DOS memory block either way. */

#include <errno.h>
#include <stdlib.h>

#if defined(__COMPACT__) || defined(__LARGE__)

#include <dos.h>
#include "heap16.h"

extern char __cdecl __libc16_stack_top[];

static unsigned last_seg;       /* the last arena; 0 until the first */

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

/* The word at seg:off */
#define WORD(seg, off) (*(unsigned __far *)MK_FP(seg, off))

/* The paragraphs that hold n bytes, for 0 < n <= 10000h; n = 0 stands
 * for 10000h, a stack top at the end of a 64 KiB DGROUP. */
#define PARAS(n) ((((n) - 1) >> 4) + 1)

/* Resizes the program's DOS memory block to end at offset end of segment
 * seg; nonzero if DOS did. A size that would not fit in a word fails
 * instead of wrapping to a smaller block. */
static int set_end(unsigned seg, unsigned end)
{
    unsigned below = seg - _psp;
    unsigned paras = below + PARAS(end);

    return paras > below && set_block(_psp, paras);
}

void * __cdecl malloc(size_t size)
{
    unsigned seg, off, end, have, next, need;
    unsigned free_seg = 0, free_off = 0;    /* the first free block passed */
    unsigned last = 0;          /* a free block ending at the heap's end */

    /* HEAP_MAX keeps size + 3 from overflowing too. */
    if (size > HEAP_MAX)
        goto fail;
    need = (size + 3) & ~1u;    /* the header, rounded up to even */
    if (last_seg) {
        seg = __libc16_heap_seg;
        off = __libc16_heap_off;
        for (;;) {
            end = WORD(seg, 0);
            if (off == end) {
                if (seg == last_seg)
                    break;
                seg += PARAS(end);
                off = 2;
                continue;
            }
            have = WORD(seg, off);
            if (have & 1) {
                off += have - 1;
                continue;
            }
            for (;;) {
                next = off + have;
                if (next == end || (WORD(seg, next) & 1))
                    break;
                have += WORD(seg, next);
            }
            if (have >= need) {
                if (have != need)
                    WORD(seg, off + need) = have - need;
                goto found;
            }
            WORD(seg, off) = have;
            if (!free_seg) {
                free_seg = seg;
                free_off = off;
            }
            if (next == end && seg == last_seg)
                last = off;
            off = next;
        }

        /* seg is the last arena and end its end: grow it if the block
         * stays within ARENA_END. */
        off = last ? last : end;
        if (need <= ARENA_END - off) {
            if (!set_end(seg, off + need))
                goto fail;
            WORD(seg, 0) = off + need;
            goto found;
        }
        seg += PARAS(end);
        if (seg < last_seg)     /* past the top of memory */
            goto fail;
    } else
        seg = dgroup() + PARAS((unsigned)__libc16_stack_top);

    /* A new arena at seg, holding just this block. */
    if (!set_end(seg, 2 + need))
        goto fail;
    WORD(seg, 0) = 2 + need;
    off = 2;
    last_seg = seg;

found:
    WORD(seg, off) = need | 1;
    if (free_seg) {
        __libc16_heap_seg = free_seg;
        __libc16_heap_off = free_off;
    } else {
        /* Nothing before the block is free, so the search can start
         * after it next time. */
        __libc16_heap_seg = seg;
        __libc16_heap_off = off + need;
    }
    return MK_FP(seg, off + 2);

fail:
    errno = ENOMEM;
    return NULL;
}

#else

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

#endif
