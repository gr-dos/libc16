# Spec: strings and memory

**Header:** `<string.h>`  
**Members:** one per function  
**Tests:** `tests/string` (`strupr` in `tests/string/upper`, `memchr` in `tests/string/memchr`, the copies and fills in `tests/string/copy`), `tests/memperf` (their time)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- C89 7.11 (string handling): `memchr`, `memcmp`, `memcpy`, `memmove`, `memset`, `strcat`, `strchr`, `strcmp`, `strcpy`, `strerror`, `strlen`, `strncat`, `strncmp`, `strncpy`, `strpbrk`, `strrchr`, `strcspn`, `strstr`.
- Microsoft C 5.1 run-time library reference, for the Microsoft names `strdup`, `stricmp`, `strlwr`, `strupr` and `strnicmp`; Microsoft C 6.0's and Open Watcom's documentation of `_fmemcpy`.
- `docs/spec/io.md` for the `errno` values `strerror` describes.

## Standard functions

Full C89 semantics. In particular: comparisons (`memcmp`, `strcmp`, `strncmp`) compare as `unsigned char` and return a negative, zero or positive `int`; `strchr` and `strrchr` find the terminating null when `c` is `'\0'`; `memcpy` of overlapping objects is undefined, and `memmove` copies as if through a temporary; `strncpy` pads with nulls to `n` bytes and does not terminate a source of `n` or more characters; `strncat` appends at most `n` characters and always a terminator; `strpbrk` returns the first character of `s` that is in `accept`, or a null pointer; `strcspn(s, reject)` is the length of the initial part of `s` containing no character of `reject` (`strlen(s)` if `reject` is empty); `strstr(s, find)` returns the first occurrence of the string `find` in `s`, `s` itself if `find` is empty, or a null pointer; `memchr(s, c, n)` returns a pointer to the first of the `n` bytes at `s` equal to `c` converted to `unsigned char`, or a null pointer: it examines exactly those bytes, stopping at none of them, so a null byte is found like any other and nothing past the `n`th is read; `n` of 0 is valid for the `mem` functions and the `strn` functions.

## Microsoft functions

- **`char *strdup(const char *s)`:** returns a copy of `s` in memory from `malloc` (`docs/spec/heap.md`), or a null pointer if `malloc` fails.
- **`int stricmp(const char *s1, const char *s2)`:** as `strcmp`, after converting each character from `A`–`Z` to `a`–`z`; no other characters are changed.
- **`char *strlwr(char *s)`:** converts `A`–`Z` in `s` to `a`–`z` in place and returns `s`.
- **`char *strupr(char *s)`:** converts `a`–`z` in `s` to `A`–`Z` in place and returns `s`; no other characters are changed, bytes from 80h up included.
- **`int strnicmp(const char *s1, const char *s2, size_t n)`:** as `strncmp`, comparing as `stricmp` does.
- **`void __far *_fmemcpy(void __far *dst, const void __far *src, size_t n)`:** `memcpy` with far pointers, in every model; the objects need not be in `DGROUP`.

## Copies and fills

`memcpy`, `memmove`, `memset` and `_fmemcpy` move a word at a time with the 8086 string instructions: `rep movsw` (or `rep stosw`) for `n / 2` words, then one `movsb` (`stosb`) when `n` is odd (#42). Each is in assembly. Their results are the same as a byte loop's, `n` of 0 included, which moves nothing; `tests/string/copy` covers every `n` up to 40 at every alignment of source and destination, `memmove`'s overlaps in both directions at every distance up to 7, and `n` above 32767 (*tested*).

- **Registers.** `DS:SI` is the source and `ES:DI` the destination. `ES` is changed. In the small model `ES` is set to `DS` and `DS` is kept, `_fmemcpy` restoring it after loading the source's segment; with far data `DS` is left with the source's segment, which the ABI allows (`docs/ABI.md`, "Registers"). `SI`, `DI` and `BP` are kept. `DS` in the small model is *tested*.
- **Direction flag.** `memmove` copies upwards when `dst`'s offset is at most `src`'s, and otherwise downwards with the direction flag set (`std`): the odd last byte first, then the words from the last down. It clears the flag (`cld`) before returning. The others clear it before copying. The flag is clear on return from every one (*tested*). An interrupt handler that runs during a downward `memmove` finds the flag set, as with any code that uses `std`, and must clear it itself if it uses string instructions.
- **Alignment.** The destination is not aligned first. On an 8086 a word move to or from an odd address takes an extra bus cycle, but is still faster than two byte moves; on an 8088 or V20, whose bus is 8 bits wide, alignment makes no difference.
- **Time.** `tests/memperf` times 20,000 bytes moved or filled 20 times by each member and by a byte loop compiled from C, the way 0.18's members copied; each member must take at most half the loop's time (*tested*, under DOSBox-X).

## `char *strerror(int errnum)`

Returns a pointer to a constant message; the caller must not modify it.

| `errnum` | Message |
|---|---|
| 0 | `No error` |
| `ENOENT` | `No such file or directory` |
| `E2BIG` | `Arg list too long` |
| `ENOEXEC` | `Exec format error` |
| `EBADF` | `Bad file number` |
| `ENOMEM` | `Not enough memory` |
| `EACCES` | `Permission denied` |
| `EEXIST` | `File exists` |
| `EXDEV` | `Cross-device link` |
| `EINVAL` | `Invalid argument` |
| `EMFILE` | `Too many open files` |
| `ENOSPC` | `No space left on device` |
| `EDOM` | `Math argument` |
| `ERANGE` | `Result too large` |
| any other | `Unknown error` |

The texts are the traditional Unix ones, which Microsoft C also uses.

## Constraints

All are `__cdecl`; each is its own member, so a program links only those it calls. 8086 instructions only.
