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
- Otherwise returns a pointer to a block of at least `size` bytes whose first bytes, up to the smaller of the old and new sizes, are the old block's; the rest are unspecified. The block may stay where it is (shrinking, or growing into a free neighbour or into the end of the heap) or move, in which case the old block is freed.
- On failure returns a null pointer, sets `errno` to `ENOMEM`, and leaves the old block allocated and unchanged.
- `realloc(p, 0)` returns either a null pointer, having freed `p`, or a unique pointer as `malloc(0)` does; the implementation documents which. libc16 returns the unique pointer: `p` itself, shrunk to no bytes and still allocated, to be passed to `free` (or `realloc`) later.

## `void *calloc(size_t n, size_t size)`

C99 7.20.3.1: allocates `n * size` bytes, all zero. If `n * size` overflows `size_t`, or the allocation fails, returns a null pointer with `errno` set to `ENOMEM`.

## Constraints

- All are `__cdecl` (`docs/ABI.md`). A program that does not call `malloc`, `calloc` or `realloc` links none of the heap; one that calls only `malloc` and `free` does not link `realloc` or `calloc`.
- Per-allocation overhead is at most 2 bytes plus alignment in the small model and at most 4 bytes plus alignment with far data. The code is small rather than fast (first fit is enough), but with far data a program that makes thousands of small allocations (an editor's lines) must not take time quadratic in their number for each one.
- 8086 instructions only (`scripts/check-8086`).

## Tests

`tests/heap` covers, in every model: allocations are distinct, aligned and writable without disturbing each other; `free` and reuse; merging of neighbours; exhaustion returning a null pointer with `ENOMEM`, then recovery after `free`; `malloc(0)`; a program built with a larger `option stack` still gets a working heap above its stack; `realloc` growing, shrinking, moving with the contents kept, failing with the old block intact, and from a null pointer; `calloc` zeroing and its overflow check. With far data: more than 64 KiB in total can be allocated, a request over the limit fails, and every allocation's offset plus size is at most 10000h.
