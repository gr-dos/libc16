/* free - libc16 (docs/spec/heap.md)
 *
 * Releases the block whose header is the word before p (crt/heap16.h):
 * merges it with the block after it if that is free, and with the block
 * before it if its PFREE bit says that is free, found by the footer at
 * its end; then writes the merged block's header and footer, marks the
 * block after it, and puts it on its size class's list. Also defines the
 * free lists and the helpers that malloc and realloc share with it, so
 * that free alone links no other member. */

#include <stdlib.h>
#include "heap16.h"

struct hfree *__libc16_heap_list[HEAP_CLASSES];
unsigned __libc16_heap_fit[HEAP_CLASSES];
unsigned __libc16_heap_map[(HEAP_CLASSES + 15) / 16];

unsigned __libc16_heap_class(unsigned size)
{
    unsigned c = SMALL_CLASSES + 1;

    if (size < 64)
        return (size - MIN_BLOCK) >> 2;
    for (; size >= 128; size >>= 1)
        c += 2;
    return size < 96 ? c - 1 : c;
}

void __libc16_heap_unlink(struct hfree *b)
{
    struct hfree *next = b->next;

    b->prev->next = next;
    if (next)
        next->prev = b->prev;
}

void __cdecl free(void *p)
{
    unsigned *b, *n, size, c;
    struct hfree *f, *h;

    if (!p)
        return;
    b = (unsigned *)p - 1;
    size = *b & ~(USED | PFREE);
    n = AT(b, size);
    if (!(*n & USED)) {
        /* A free block's header is its size: the block before it, this
         * one, is allocated. */
        if (*n >= MIN_BLOCK)
            __libc16_heap_unlink((struct hfree *)n);
        size += *n;
    }
    if (*b & PFREE) {
        b = AT(b, -b[-1]);
        if (*b >= MIN_BLOCK)
            __libc16_heap_unlink((struct hfree *)b);
        size += *b;
    }
    *b = size;
    n = AT(b, size);
    n[-1] = size;
    *n |= PFREE;
    if (size >= MIN_BLOCK) {
        c = __libc16_heap_class(size);
        if (size >= ~__libc16_heap_fit[c])
            __libc16_heap_fit[c] = 0;
        __libc16_heap_map[c >> 4] |= 1u << (c & 15);
        /* The pseudo-block whose next link is the list's head */
        h = (struct hfree *)((char *)&__libc16_heap_list[c] - offsetof(struct hfree, next));
        f = (struct hfree *)b;
        f->prev = h;
        f->next = h->next;
        if (f->next)
            f->next->prev = f;
        h->next = f;
    }
}
