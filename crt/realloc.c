/* realloc - libc16 (docs/spec/heap.md)
 *
 * Works on the block header that malloc and free share (crt/malloc.c):
 * the word before p holds the block's size, header included, with bit 0
 * set while it is allocated. A request that fits in the block keeps it
 * where it is and gives the rest back, as a block of its own that free
 * releases; any larger one moves to a block from malloc, so the old block
 * is untouched if that fails (C99 7.20.3.4). realloc(p, 0) is the first
 * case: p stays allocated, shrunk to no bytes, unique as malloc(0) is. */

#include <stdlib.h>
#include <string.h>

void * __cdecl realloc(void *p, size_t size)
{
    unsigned have, need;
    char *rest;
    void *q;

    if (!p)
        return malloc(size);
    have = ((unsigned *)p)[-1] & ~1u;
    if (size <= have - 2) {
        /* size fits, so it is at most 65,533 and need cannot overflow. */
        need = (size + 3) & ~1u;
        if (need != have) {
            ((unsigned *)p)[-1] = need | 1;
            rest = (char *)p - 2 + need;
            *(unsigned *)rest = (have - need) | 1;
            free(rest + 2);
        }
        return p;
    }
    q = malloc(size);
    if (q) {
        memcpy(q, p, have - 2);
        free(p);
    }
    return q;
}
