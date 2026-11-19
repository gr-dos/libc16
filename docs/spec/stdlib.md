# Spec: sorting, random numbers and abnormal termination

**Header:** `<stdlib.h>`  
**Members:** `qsort`, `rand`, `srand`, `abort`, in each of the small, compact and large libraries (decision 11)  
**Tests:** `tests/stdlib` (run by `tests/run-dos`, in every model)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- C89 7.10.5.2 (`qsort`), 7.10.2.1 and 7.10.2.2 (`rand`, `srand`) and 7.10.4.1 (`abort`).
- The generator's multiplier and increment, 1664525 and 1013904223: the linear congruential generator modulo 2³² that *Numerical Recipes in C* (2nd ed., section 7.1, "An Even Quicker Generator") gives, after Knuth's *The Art of Computer Programming*, vol. 2, section 3.2.1, whose theorem A says it has the full period 2³².
- gr-dos/perl16's `docs/M3-RUNTIME-AUDIT.md`, section 7.10, on why the high bits are returned.
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

## `int rand(void)`, `void srand(unsigned seed)`

C89 7.10.2: `rand` returns a pseudo-random `int` from 0 to `RAND_MAX`, which `<stdlib.h>` defines as 32767, and `srand` starts a new sequence; the same seed gives the same sequence. Before any call of `srand` the sequence is that of `srand(1)`.

The generator is a 32-bit linear congruential one, fixed so that sequences are the same in every release and model:

- the state is an `unsigned long`, *x*;
- `srand(seed)` sets *x* to `seed` (zero-extended);
- `rand()` sets *x* to (1664525 *x* + 1013904223) mod 2³² and returns bits 31 to 17 of the new *x*, that is *x* >> 17, from 0 to 32767.

C89's example generator returns bits 30 to 16 of a similar sequence; the low bits of such a generator have short periods (bit *k* has period 2^(*k*+1)), so the highest bits are the ones returned. The period of *x* is 2³² for every seed.

`rand` and `srand` are separate members sharing the state, so a program that never calls `srand` does not link it. Neither links any other member except the compiler's 32-bit multiply (`docs/spec/arith.md`), if they use it.

## `void abort(void)`

Ends the program abnormally (C89 7.10.4.1), as Microsoft C does: writes `abnormal program termination` and `\r\n` to handle 2 (standard error) with INT 21h function 40h, then terminates with INT 21h function 4Ch with exit code 3. It does not return. It runs nothing else first: not the `atexit` functions or the termination table (`docs/spec/startup.md`), which C89 7.10.4.1 does not call; and it does not write the streams' pending output, which is lost (C89 leaves that to the implementation; `docs/spec/stdio.md`, "Termination"), so `tests/stdbuf/bufabrt` checks it is lost. A failure to write the message is ignored.

First, if a `SIGABRT` handler is set, `abort` resets the disposition to `SIG_DFL` and calls it; if it returns, `abort` goes on as above (`docs/spec/signal.md`, "`abort`").

`abort` references no other member (only the communal variable `___libc16_sigabrt`, `docs/spec/signal.md`), and does not need the startup (`docs/spec/startup.md`, "Programs without the startup").

## Constraints

- Both are `__cdecl` (`docs/ABI.md`). 8086 instructions only.
- `qsort` links no other member: in particular no heap and no 32-bit helper.
- `atexit` is specified with `exit` in `docs/spec/startup.md`, since they share the termination table.

## Tests

`tests/stdlib/qsort` sorts `int`s, 7-byte records and 300-byte records: empty, one, two and 1,000 elements; sorted, reversed, all equal, few distinct values and pseudo-random. It checks that each result is ordered and a permutation of the input, that `compar` sees only pointers to elements, and the number of calls of `compar` against 3 *n* ⌈log₂ *n*⌉. An inconsistent `compar` must return with a permutation. `tests/stdlib/rand` checks `RAND_MAX`, the first values of the sequence with no `srand`, after `srand(1)`, `srand(0)`, `srand(12345)` and `srand(65535)` (computed on the host from the formula above), that `srand` with the same seed repeats them, and that 10,000 values are all from 0 to 32767 and spread over the range (each eighth of it gets 1,000 to 1,500) with both odd and even values.

`tests/stdlib/abort` registers an `atexit` function, makes handle 2 a file, calls `abort`, and checks its exit code and that the function did not run; `abortchk` then checks the file's contents. The `atexit` tests are listed in `docs/spec/startup.md`.
