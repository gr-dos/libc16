# Spec: heap

**Header:** `<stdlib.h>`  
**Members:** `malloc`, `free`, `realloc`, `calloc`, and whatever private members they share, in each of the small, compact and large libraries (decision 11)  
**Tests:** `tests/heap` (run by `tests/run-dos`, in every model)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- C99 7.20.3 (memory management functions).
- INT 21h function 4Ah (resize memory block), DOS 2.0: Ralf Brown's Interrupt List; MS-DOS 2.0 `SYSCALL.DOC` (SetBlock).
- `docs/spec/startup.md` for the layout of `DGROUP` and the memory block the startup leaves.
- `docs/ABI.md`, "Memory models", for pointers and registers in each model.

## Layout

The startup places the stack at the top of the program's image in `DGROUP` and shrinks the program's DOS memory block to end at the top of the stack, rounded up to a paragraph (`___libc16_stack_top`). The heap lies above the stack and grows the memory block with INT 21h function 4Ah as it needs more. If the startup does not make something the heap needs available, the implementation may add a symbol to `crt/cstart.asm`, updating `docs/spec/startup.md` and its tests; a program that does not call the heap functions must not change in size.

- **Small model.** Pointers are near, so the heap is in `DGROUP` above the stack and never grows the memory block beyond the end of a 64 KiB `DGROUP`.
- **Compact and large models.** Pointers are far, so the heap may use all the memory DOS lets the block grow to above the stack, beyond `DGROUP` and across as many 64 KiB segments as that takes. Each allocation is one object inside a single segment: its far pointer's offset plus its size is at most 10000h, so the compiler's far pointer arithmetic (on the offset only) works anywhere inside it. Requests of up to 65,000 bytes must be supported; the implementation documents its exact limit (less than 65,536 bytes), and a larger request fails. libc16's limit is 65,530 bytes: the heap is a run of arenas of at most one segment each, and an allocation's 2-byte header and its arena's 2-byte end word leave at most 65,530 of an arena's 65,534 bytes.

## `void *malloc(size_t size)`

- Returns a pointer to at least `size` bytes, aligned to 2 bytes, not overlapping any other live allocation, the stack, or static data; the contents are unspecified.
- `malloc(0)` returns a unique pointer that can be passed to `free`.
- Returns a null pointer and sets `errno` to `ENOMEM` when the request cannot be met: no free block is large enough and the memory block cannot grow (another DOS block lies above it, or, in the small model, `DGROUP` would pass 64 KiB), the request is too large for the model, or the size with overhead overflows.

## `void free(void *p)`

- Makes the block allocated by `malloc`, `calloc` or `realloc` available to later calls. `free(NULL)` does nothing. Any other pointer not returned by them, or freed twice, is undefined.
- Adjacent free blocks are merged, so that freeing every allocation lets one request of the total size succeed (with far data, as long as that size is allowed and fits in one segment).
- Memory need not be returned to DOS.

## `void *realloc(void *p, size_t size)`

C99 7.20.3.4:

- `realloc(NULL, size)` is `malloc(size)`.
- Otherwise returns a pointer to a block of at least `size` bytes whose first bytes, up to the smaller of the old and new sizes, are the old block's; the rest are unspecified.
- The block stays where it is when it can: when `size` fits in it (what it no longer needs is freed, as a block of its own that later requests can have); when the free block after it, if there is one, together with it holds `size` (the rest of that free block stays free); and when it is the last block of the heap, with far data the last block of the last arena, possibly followed by a free block, and the memory block can grow to hold `size` there (with far data, while the arena stays within its segment). Otherwise it moves, and the old block is freed.
- On failure returns a null pointer, sets `errno` to `ENOMEM`, and leaves the old block allocated and unchanged.
- `realloc(p, 0)` returns either a null pointer, having freed `p`, or a unique pointer as `malloc(0)` does; the implementation documents which. libc16 returns the unique pointer: `p` itself, shrunk to no bytes and still allocated, to be passed to `free` (or `realloc`) later.

## `void *calloc(size_t n, size_t size)`

C99 7.20.3.1: allocates `n * size` bytes, all zero. If `n * size` overflows `size_t`, or the allocation fails, returns a null pointer with `errno` set to `ENOMEM`.

## Constraints

- All are `__cdecl` (`docs/ABI.md`). A program that does not call `malloc`, `calloc` or `realloc` links none of the heap; one that calls only `malloc` and `free` does not link `realloc` or `calloc`.
- Space: a block holding `n` bytes takes at most the larger of `n + 2` rounded up to a multiple of 4 and 8 bytes (12 with far data). Each arena takes at most 4 bytes besides its blocks. The heap's own static data in `DGROUP` is at most 256 bytes. Free memory needs no allocation of its own: what the heap keeps about free blocks lives in them.
- Time: see "Performance".
- 8086 instructions only (`scripts/check-8086`).

## Performance

Programs keep thousands of blocks (an editor's lines, an interpreter's strings, hash entries and arrays) and free and reallocate them in any order (gr-dos/libc16#30, from Perl 3 in `gr-dos/perl16`). Each call's time must not grow with the number of blocks in the heap, allocated or free, except as amortized over many calls:

- `malloc` does not pass every block, or every free block, looking for one that fits. Free blocks that cannot hold a request, however many, cost it nothing after the first such request of that size, until a larger block is freed.
- `free` merges a block with its free neighbours without a search.
- `realloc` grows a block in place as "realloc" above requires, so growing the last block of the heap takes no copy, and a block that moves costs what `malloc`, the copy and `free` cost.

Work proportional to the number of distinct block sizes (a few dozen size classes) is allowed; so is copying a moved block's bytes.

`tests/heapperf/scale.c` checks this in every model. Each workload runs `n` blocks at a time repeated `r` times, then `2n` blocks repeated `r/2` times: the same number of calls on a heap holding twice the blocks. Time is in BIOS ticks (18.2 per second) of DOSBox-X's emulated 8086 at its fixed rate (`tests/run-dos`), which counts emulated work rather than the host's speed. A heap whose calls take time independent of the number of blocks takes about as long for the second run; one whose calls each search all the blocks takes twice as long (the whole workload four times as long for twice the blocks, as #30 measured). The check allows the second run at most 3/2 of the first plus 2 ticks, for the clock's granularity:

| Workload | Calls per repetition | Free blocks during it |
|---|---|---|
| `small` | `n` `malloc` of 1 to 24 bytes, then `n` `free` | none, then up to `n` |
| `holes` | `2n` `malloc(8)`, `n` `free` of every other one, `n` `malloc(24)`, which none of the holes can hold, then `3n` `free` | `n` |
| `grow` | `n` `malloc(8)`, then `realloc` of each to 24 and then 40 bytes; each is followed by the next, so it moves, leaving a hole | up to `n` |
| `append` | one block grown by 8 bytes `n` times by `realloc`, nothing after it | none |
| `table` | per key, `malloc(12)` and `malloc(4 to 16)` that stay, a `malloc(16 to 48)` freed at once, and a bucket array doubled by `realloc` | a few |

`scale WORKLOAD N R` measures one workload with other sizes. With libc16 0.12 (first fit; with far data the search starts at the first block that may be free), in DOSBox-X's emulated 8086, every model:

| Workload | `n` × `r` | small | compact | large |
|---|---|---|---|---|
| `small` | 100 × 16, 200 × 8 | 5, 9 | 2, 1 | 2, 1 |
| `holes` | 100 × 16, 200 × 8 | 48, 95 | 49, 94 | 49, 94 |
| `grow` | 100 × 16, 200 × 8 | 20, 34 | 20, 30 | 20, 31 |
| `append` | 200 × 8, 400 × 4 | 64, 126 | 64, 126 | 64, 127 |
| `table` | 100 × 16, 200 × 8 | 27, 50 | 38, 72 | 39, 72 |

Every search in the small model starts at the heap's first block, so even `small` grows with the number of blocks; with far data it starts at the first block that may be free, which helps only while nothing before it is freed. `holes`, `grow` and `table` pass every hole on every call, and `append` copies the whole block on every call. The checked sizes (`n` of 500 to 1,000) take minutes each with 0.12, past `tests/run-dos`'s limit.

## Tests

`tests/heap` covers, in every model: allocations are distinct, aligned and writable without disturbing each other; `free` and reuse; merging of neighbours; exhaustion returning a null pointer with `ENOMEM`, then recovery after `free`; `malloc(0)`; a program built with a larger `option stack` still gets a working heap above its stack; `realloc` growing in place at the end of the heap and into a free neighbour, shrinking, moving with the contents kept, failing with the old block intact, and from a null pointer; `calloc` zeroing and its overflow check; thousands of small allocations; and a long pseudo-random sequence of `malloc`, `realloc` and `free` (`random.c`) whose blocks never overlap and keep their contents, and whose free blocks have all merged once everything is freed. With far data: more than 64 KiB in total can be allocated, a request over the limit fails, and every allocation's offset plus size is at most 10000h. `tests/heapperf` checks "Performance".
