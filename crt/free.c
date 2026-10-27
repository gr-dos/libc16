/* free - libc16 (docs/spec/heap.md)
 *
 * Marks the block free by clearing bit 0 of its header word, the word
 * before p (crt/malloc.c). malloc merges free neighbours as it searches.
 * With far data free also moves malloc's starting point back to the
 * block if it is before it (crt/heap16.h), and defines that point, so
 * that free alone links no other member. */

#include <stdlib.h>

#if defined(__COMPACT__) || defined(__LARGE__)

#include <dos.h>
#include "heap16.h"

unsigned __cdecl __libc16_heap_seg;
unsigned __cdecl __libc16_heap_off;

void __cdecl free(void *p)
{
    unsigned seg, off;

    if (p) {
        ((unsigned *)p)[-1] &= ~1u;
        /* Arenas are in increasing segments, so blocks are ordered by
         * segment, then offset. */
        seg = FP_SEG(p);
        off = FP_OFF(p) - 2;
        if (seg < __libc16_heap_seg || (seg == __libc16_heap_seg && off < __libc16_heap_off)) {
            __libc16_heap_seg = seg;
            __libc16_heap_off = off;
        }
    }
}

#else

void __cdecl free(void *p)
{
    if (p)
        ((unsigned *)p)[-1] &= ~1u;
}

#endif
