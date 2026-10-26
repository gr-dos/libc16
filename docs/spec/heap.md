# Spec: heap

**Header:** `<stdlib.h>`  
**Members:** `malloc`, `free`, and whatever private members they share  
**Tests:** `tests/heap` (run by `tests/run-dos`)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- C99 7.20.3 (memory management functions).
- INT 21h function 4Ah (resize memory block), DOS 2.0: Ralf Brown's Interrupt List; MS-DOS 2.0 `SYSCALL.DOC` (SetBlock).
- `docs/spec/startup.md` for the layout of `DGROUP` and the memory block the startup leaves.

## Layout

The small model has one 64 KiB data segment, `DGROUP`, and near (16-bit) pointers. The startup places the stack at the top of the program's image in `DGROUP` and shrinks the program's DOS memory block to end at the top of the stack. The heap lies in `DGROUP` above the stack: it grows the memory block with INT 21h function 4Ah as it needs more, never beyond the end of a 64 KiB `DGROUP`. If the startup does not make the top of the stack available, the implementation may add a symbol to `crt/cstart.asm` for it, updating `docs/spec/startup.md` and its tests; a program that does not call `malloc` must not change in size.

## `void *malloc(size_t size)`

- Returns a pointer to at least `size` bytes, aligned to 2 bytes, not overlapping any other live allocation, the stack, or static data; the contents are unspecified.
- `malloc(0)` returns a unique pointer that can be passed to `free`.
- Returns a null pointer and sets `errno` to `ENOMEM` when the request cannot be met: no free block is large enough and the memory block cannot grow (another DOS block lies above it, or `DGROUP` would pass 64 KiB), or the size with overhead overflows.

## `void free(void *p)`

- Makes the block allocated by `malloc` available to later calls. `free(NULL)` does nothing. Any other pointer not returned by `malloc`, or freed twice, is undefined.
- Adjacent free blocks are merged, so that freeing every allocation lets one request of the total size succeed.
- Memory need not be returned to DOS.

## Constraints

- Both are `__cdecl` (`docs/ABI.md`). A program that does not call `malloc` links none of the heap.
- Per-allocation overhead is at most 2 bytes plus alignment; the code is small rather than fast (first fit is enough).
- 8086 instructions only (`scripts/check-8086`).

## Tests

`tests/heap` covers: allocations are distinct, aligned and writable without disturbing each other; `free` and reuse; merging of neighbours; exhaustion returning a null pointer with `ENOMEM`, then recovery after `free`; `malloc(0)`; a program built with a larger `option stack` still gets a working heap above its stack.
