/* heap16.h - private to libc16's heap with far data (crt/malloc.c,
 * crt/free.c; docs/spec/heap.md, "Layout")
 *
 * In the compact and large models the heap is a run of arenas above the
 * stack, each one segment, one after another in the program's DOS memory
 * block. An arena starts at offset 0 of its segment with a word holding
 * its end: the offset just past its last block. Its blocks follow from
 * offset 2, laid out as in the small model: a header word with the
 * block's size in bytes, header included, which is even, with bit 0 set
 * while the block is allocated. The next arena starts at the paragraph
 * after an arena's end, so only the last arena can grow. */

#ifndef LIBC16_HEAP16_H
#define LIBC16_HEAP16_H

/* The highest end of an arena. Every offset in an arena then fits in a
 * word without wrapping, and a block's offset plus its size is at most
 * 10000h (the spec's Layout). */
#define ARENA_END 0xFFFEu

/* The largest request: an arena's end word and the block's header leave
 * ARENA_END - 4 bytes, 65,530 (docs/spec/heap.md). */
#define HEAP_MAX (ARENA_END - 4)

/* The first block that may be free, as the segment of its arena and the
 * offset of its header: every block before it is allocated, so malloc
 * starts its search there instead of at the first arena. free lowers it;
 * malloc moves it on. 0:0 until the first malloc. Defined in crt/free.c,
 * so that free links nothing else. */
extern unsigned __cdecl __libc16_heap_seg;
extern unsigned __cdecl __libc16_heap_off;

#endif
