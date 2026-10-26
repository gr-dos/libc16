/* free - libc16 (docs/spec/heap.md)
 *
 * Marks the block free by clearing bit 0 of its header word, the word
 * before p (crt/malloc.c). malloc merges free neighbours as it searches. */

#include <stdlib.h>

void __cdecl free(void *p)
{
    if (p)
        ((unsigned *)p)[-1] &= ~1u;
}
