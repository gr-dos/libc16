# Spec: character classes

**Header:** `<ctype.h>`  
**Members:** one per function  
**Tests:** `tests/ctype` (run by `tests/run-dos`, in every model)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- C89 7.3 (character handling) and 5.2.1 (the basic character sets), in the "C" locale.

## Functions

`isalnum`, `isalpha`, `iscntrl`, `isdigit`, `isgraph`, `islower`, `isprint`, `ispunct`, `isspace`, `isupper`, `isxdigit`, `tolower` and `toupper`, with C89 semantics in the "C" locale, which is the only locale. Each takes an `int` whose value is `EOF` or representable as an `unsigned char`; any other value is undefined in C89, and libc16 treats it as not belonging to any class (and `tolower` and `toupper` return it unchanged).

| Function | True for |
|---|---|
| `isdigit` | `0`–`9` |
| `isxdigit` | `0`–`9`, `a`–`f`, `A`–`F` |
| `islower`, `isupper` | `a`–`z`; `A`–`Z` |
| `isalpha`, `isalnum` | letters; letters and digits |
| `isspace` | space, `\t`, `\n`, `\v`, `\f`, `\r` |
| `iscntrl` | 00h–1Fh and 7Fh |
| `isprint` | 20h–7Eh |
| `isgraph` | 21h–7Eh |
| `ispunct` | 21h–7Eh except letters and digits |

The classification functions return non-zero for true (not necessarily 1) and 0 for false. Bytes 80h–FFh and `EOF` are in no class. `tolower` converts `A`–`Z` to `a`–`z` and returns any other value unchanged; `toupper` the reverse.

## Constraints

- Functions only, with no macros and no table, so that a program links only the functions it calls and no 256-byte table; each is a few instructions.
- All are `__cdecl` (`docs/ABI.md`). 8086 instructions only.

## Tests

`tests/ctype` checks every function against the table above for every value from `EOF` to 255.
