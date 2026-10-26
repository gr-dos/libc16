# Spec: strings and memory

**Header:** `<string.h>`  
**Members:** one per function  
**Tests:** `tests/string`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- C89 7.11 (string handling): `memcmp`, `memcpy`, `memset`, `strcat`, `strchr`, `strcmp`, `strcpy`, `strerror`, `strlen`, `strncmp`, `strrchr`.
- Microsoft C 5.1 run-time library reference, for the Microsoft names `strdup`, `stricmp` and `strlwr`.
- `docs/spec/io.md` for the `errno` values `strerror` describes.

## Standard functions

Full C89 semantics. In particular: comparisons (`memcmp`, `strcmp`, `strncmp`) compare as `unsigned char` and return a negative, zero or positive `int`; `strchr` and `strrchr` find the terminating null when `c` is `'\0'`; `memcpy` of overlapping objects is undefined; `n` of 0 is valid for the `mem` functions and `strncmp`.

## Microsoft functions

- **`char *strdup(const char *s)`:** returns a copy of `s` in memory from `malloc` (`docs/spec/heap.md`), or a null pointer if `malloc` fails.
- **`int stricmp(const char *s1, const char *s2)`:** as `strcmp`, after converting each character from `A`–`Z` to `a`–`z`; no other characters are changed.
- **`char *strlwr(char *s)`:** converts `A`–`Z` in `s` to `a`–`z` in place and returns `s`.

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
