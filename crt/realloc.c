/* realloc - libc16 (docs/spec/heap.md)
 *
 * Works on the blocks malloc and free share (crt/heap16.h). The block
 * stays where it is when the request fits in it, when it fits in it and
 * the free block after it, and when the block reaches the heap's end,
 * possibly through a free block, and the heap can grow there; what it
 * then holds beyond the request is freed as a block of its own. Any
 * other request moves to a block from malloc, so the old block is
 * untouched if that fails (C99 7.20.3.4). realloc(p, 0) is the first
 * case: p stays allocated, shrunk to no bytes, unique as malloc(0) is. */

#include <stdlib.h>
#include <string.h>
#include "heap16.h"

void * __cdecl realloc(void *p, size_t size)
{
    unsigned *b, *n, have, need, next;
    void *q;

    if (!p)
        return malloc(size);
    b = (unsigned *)p - 1;
    have = *b & ~(USED | PFREE);
    /* A request too large for any block moves, and malloc fails. */
    need = size > HEAP_MAX ? (unsigned)-1 : (size + 5) & ~3u;
    if (need < MIN_BLOCK)
        need = MIN_BLOCK;
    if (need > have) {
        n = AT(b, have);
        next = *n & USED ? 0 : *n;      /* the free block after it */
        if (need <= have + next)
            have += next;
        else if (AT(n, next) == __libc16_heap_end && __libc16_heap_grow(b, need))
            have = need;
        else {
            q = malloc(size);
            if (q) {
                memcpy(q, p, have - 2);
                free(p);
            }
            return q;
        }
        if (next >= MIN_BLOCK)
            __libc16_heap_unlink((struct hfree *)n);
        *AT(b, have) &= ~PFREE;
    }
    *b = (*b & PFREE) | need | USED;
    if (have != need) {
        n = AT(b, need);
        *n = (have - need) | USED;
        free(n + 1);
    }
    return p;
}
