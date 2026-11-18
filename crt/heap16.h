/* heap16.h - private to libc16's heap (crt/malloc.c, crt/free.c,
 * crt/realloc.c; docs/spec/heap.md)
 *
 * Blocks. Every block starts with a header word: its size in bytes,
 * header included, a multiple of 4, with USED set while it is allocated
 * and PFREE set while the block before it is free. A free block also ends
 * with a footer word holding its size, so that free finds the block
 * before one it releases without a search (boundary tags, as in Knuth's
 * TAOCP vol. 1, 2.5), and free neighbours are always merged, so a free
 * block never follows another. A free block of at least MIN_BLOCK bytes
 * holds the two links of its size class's list (struct hfree) after its
 * header; a smaller one, a fragment of 4 or (with far data) 8 bytes left
 * over by a split, is on no list and is only merged into its neighbours
 * when they are freed.
 *
 * The end. A run of blocks ends with a sentinel word, an allocated block
 * of size 0 (USED, with PFREE if the last block is free), so that free
 * never merges past it. __libc16_heap_end points to the sentinel of the
 * heap's end, where malloc and realloc grow the heap.
 *
 * Small model: one run of blocks in DGROUP from the top of the stack
 * (__libc16_stack_top, crt/cstart.asm, rounded up to even), its sentinel
 * at most at offset FFFEh.
 *
 * Compact and large models: a run of arenas, one after another in the
 * program's DOS memory block from the paragraph after the top of the
 * stack. An arena's blocks start at offset 0 of its segment, and its
 * sentinel is at most at offset ARENA_END, so an arena is at most 65,534
 * bytes and every block's offset plus size is at most 10000h. When a
 * request does not fit in the last arena, the next starts at the
 * paragraph after it: the last arena's sentinel first moves up to the
 * word before that paragraph's last word, and the bytes it passes become
 * a free block, so that an arena spends no more than its sentinel and the
 * 2 bytes after it.
 *
 * Free lists. Each free block of at least MIN_BLOCK bytes is on the
 * doubly linked list of its size class (__libc16_heap_class): one class
 * per size below 64 bytes, and two per power of two above. malloc takes
 * the first block that fits from the list of the request's class, then
 * the first block of any larger class, all of whose blocks fit. To pass
 * no block twice for nothing, __libc16_heap_fit[c] holds the complement
 * of a size that no block on list c reaches (0, all ones, when nothing is
 * known), set when a search of the list fails and cleared when a block
 * that large is put on it. The first link of a list's first block points
 * back at a pseudo-block whose next link is the list's head, so that a
 * block is unlinked without knowing its class. */

#ifndef LIBC16_HEAP16_H
#define LIBC16_HEAP16_H

#include <stddef.h>

#if defined(__COMPACT__) || defined(__LARGE__)
#define HEAP_FAR 1
#define MIN_BLOCK 12u           /* header and two far links */
#define ARENA_END 0xFFFCu       /* the highest sentinel in an arena */
#else
#define MIN_BLOCK 8u            /* header, two near links and footer */
#define ARENA_END 0xFFFEu       /* the highest sentinel in DGROUP */
#endif

/* The largest request: an arena's block from offset 0 to ARENA_END less
 * its header, 65,530 bytes (docs/spec/heap.md, "Layout"). */
#define HEAP_MAX 65530u

#define USED  1u                /* header: the block is allocated */
#define PFREE 2u                /* header: the block before it is free */

/* Classes: one per multiple of 4 from MIN_BLOCK to 60, then two per
 * power of two from 64 to 65,532 bytes. */
#define SMALL_CLASSES ((64u - MIN_BLOCK) / 4)
#define HEAP_CLASSES (SMALL_CLASSES + 20)

struct hfree {
    unsigned head;
    struct hfree *next, *prev;
};

/* The header word n bytes after block b */
#define AT(b, n) ((unsigned *)((char *)(b) + (n)))

/* In crt/free.c, so that free alone links no other member. */
extern struct hfree *__libc16_heap_list[HEAP_CLASSES];
extern unsigned __libc16_heap_fit[HEAP_CLASSES];
extern unsigned __libc16_heap_map[(HEAP_CLASSES + 15) / 16];
unsigned __libc16_heap_class(unsigned size);
void __libc16_heap_unlink(struct hfree *b);

/* In crt/malloc.c. Grows the heap's end so that block b, which reaches
 * to the end or to a free block at the end (or with far data starts a
 * new arena), is need bytes and ends at a new sentinel; 0 if it cannot. */
extern unsigned *__libc16_heap_end;
int __libc16_heap_grow(unsigned *b, unsigned need);

#endif
