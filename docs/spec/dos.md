# Spec: DOS interface, environment and paths

**Headers:** `<dos.h>`, `<stdlib.h>`  
**Members:** one per function  
**Tests:** `tests/dos`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- INT 21h functions 19h (get drive), 1Ah (set disk transfer area), 2Fh (get disk transfer area), 47h (get current directory), 4Eh (find first) and 4Fh (find next), all DOS 2.0; the environment segment at PSP offset 2Ch: Ralf Brown's Interrupt List; MS-DOS 2.0 `SYSCALL.DOC`.
- The interfaces of Microsoft C 5.1's `_dos_findfirst`, `_dos_findnext`, `int86`, `getenv` and `_fullpath`, and the layouts of `struct find_t` and `union REGS`: the Microsoft C 5.1 run-time library reference.
- C89 7.10.4.4 (`getenv`).

## `unsigned _dos_findfirst(const char *path, unsigned attr, struct find_t *f)`, `unsigned _dos_findnext(struct find_t *f)`

Find the first file matching `path` (which may contain wildcards) with attributes `attr` (`_A_*`, as INT 21h 4Eh interprets them), or the next one of the same search. `struct find_t` is DOS's search record: 21 reserved bytes, `attrib`, `wr_time`, `wr_date`, `size`, and `name` (the 8.3 name, null-terminated), 43 bytes with no padding. Return 0, or the DOS error code with `errno` set as in `docs/spec/io.md` (`ENOENT` when nothing matches). The disk transfer area is restored to its previous address before returning.

## `int int86(int intno, union REGS *in, union REGS *out)`

Loads `AX`, `BX`, `CX`, `DX`, `SI` and `DI` from `*in`, executes `INT intno`, and stores those registers and the carry flag (`cflag`, non-zero when set) in `*out`; returns `AX`. `in` and `out` may be the same. `DS`, `ES`, `BP` and the direction flag are as the caller had them during the interrupt and are restored after it.

## `char *getenv(const char *name)`

Searches the program's environment (the segment at PSP offset 2Ch: `NAME=value` strings, each null-terminated, ending with an empty string) for an entry whose name equals `name` exactly, and returns its value, or a null pointer. The environment is outside `DGROUP`, so the value is copied to a static buffer of 256 bytes, which the next call overwrites; a value that does not fit is treated as absent. The program must not modify the result.

## `char *_fullpath(char *buf, const char *path, size_t size)`

Stores in `buf` the absolute path of `path`: an upper-case drive letter, `:\`, and the directories and name, with `/` read as `\`, `.` and `..` components resolved, repeated separators collapsed, and no trailing `\` except for a root. A relative `path` (with or without a drive) starts from that drive's current directory. Returns `buf`, or a null pointer: `ERANGE` when the result and its terminator need more than `size` bytes, `ENOENT` for an invalid drive, `EINVAL` for an empty `path`. Unlike Microsoft's, a null `buf` is not supported (`EINVAL`). The case of the input is kept; the path need not exist.

## Constraints

All are `__cdecl`. 8086 instructions only.
