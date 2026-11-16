# Spec: sorting and abnormal termination

**Header:** `<stdlib.h>`  
**Members:** `qsort`, `abort`, in each of the small, compact and large libraries (decision 11)  
**Tests:** `tests/stdlib` (run by `tests/run-dos`, in every model)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- C89 7.10.5.2 (`qsort`) and 7.10.4.1 (`abort`).
- The Microsoft C 5.1 run-time library reference, for what `abort` prints and its exit code.
- INT 21h functions 40h (write) and 4Ch (terminate): Ralf Brown's Interrupt List.
- `docs/ABI.md`, "Callbacks", for the comparison function's type.

## `void qsort(void *base, size_t nmemb, size_t size, int (__cdecl *compar)(const void *, const void *))`

Sorts the array of `nmemb` elements of `size` bytes at `base` into ascending order by `compar`, as C89 says. `compar` returns a negative, zero or positive `int` as its first argument is less than, equal to or greater than its second.

- The sort is not stable: the order of elements that compare equal is unspecified.
- `nmemb` of 0 or 1, or `size` of 0, sorts nothing and calls `compar` not at all.
- `compar` is only ever passed pointers to elements of the array, never to a copy, and it may compare an element with itself.
- **Worst case.** At most a constant times *n* log₂ *n* calls of `compar` for every input of *n* elements, sorted, reversed and all-equal arrays included. There is no quadratic worst case.
- **Memory.** No heap, and stack use that does not grow with `nmemb` or `size`: elements are exchanged in place, a byte or word at a time, so any `size` works.
- **A `compar` that is not consistent** (for example one that returns arbitrary results) leaves the array in an unspecified order, but `qsort` still returns after a bounded number of calls, touches only the array, and leaves it a permutation of what it was.
- The array is one object, inside one segment: `base` plus `nmemb * size` does not pass the end of its segment. With far data (compact and large) `base` is a far pointer, and the array need not be in `DGROUP`.

## `void abort(void)`

Ends the program abnormally (C89 7.10.4.1), as Microsoft C does: writes `abnormal program termination` and `\r\n` to handle 2 (standard error) with INT 21h function 40h, then terminates with INT 21h function 4Ch with exit code 3. It does not return. It runs nothing else first; libc16 has no `atexit` and no `signal`, so there is no `SIGABRT` handler to call, and its output streams are unbuffered, so there is nothing to flush. A failure to write the message is ignored.

`abort` references no other member, and does not need the startup (`docs/spec/startup.md`, "Programs without the startup").

## Constraints

- Both are `__cdecl` (`docs/ABI.md`). 8086 instructions only.
- `qsort` links no other member: in particular no heap and no 32-bit helper.

## Tests

`tests/stdlib/qsort` sorts `int`s, 7-byte records and 300-byte records: empty, one, two and 1,000 elements; sorted, reversed, all equal, few distinct values and pseudo-random. It checks that each result is ordered and a permutation of the input, that `compar` sees only pointers to elements, and the number of calls of `compar` against 3 *n* ⌈log₂ *n*⌉. An inconsistent `compar` must return with a permutation. `tests/stdlib/abort` makes handle 2 a file, calls `abort`, and checks its exit code; `abortchk` then checks the file's contents.
