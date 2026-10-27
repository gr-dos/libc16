# Spec: DOS interface, environment and paths

**Headers:** `<dos.h>`, `<conio.h>`, `<stdlib.h>`  
**Members:** one per function  
**Tests:** `tests/dos`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- INT 21h functions 0Bh (check input status), 19h (get drive), 1Ah (set disk transfer area), 2Fh (get disk transfer area), 47h (get current directory), 4Eh (find first) and 4Fh (find next), all DOS 2.0; the environment segment at PSP offset 2Ch: Ralf Brown's Interrupt List; MS-DOS 2.0 `SYSCALL.DOC`.
- The interfaces of Microsoft C 5.1's `_dos_findfirst`, `_dos_findnext`, `int86`, `int86x`, `intdos`, `intdosx`, `segread`, `bdos`, `kbhit`, `getenv` and `_fullpath`, the `FP_SEG` and `FP_OFF` macros, and the layouts of `struct find_t`, `union REGS` and `struct SREGS`: the Microsoft C 5.1 run-time library reference. GR-DOS's ATTRIB and MEM, from the MS-DOS 4.00 source, are the call sites (`docs/CONSUMERS.md`).
- C89 7.10.4.4 (`getenv`).

## `unsigned _dos_findfirst(const char *path, unsigned attr, struct find_t *f)`, `unsigned _dos_findnext(struct find_t *f)`

Find the first file matching `path` (which may contain wildcards) with attributes `attr` (`_A_*`, as INT 21h 4Eh interprets them), or the next one of the same search. `struct find_t` is DOS's search record: 21 reserved bytes, `attrib`, `wr_time`, `wr_date`, `size`, and `name` (the 8.3 name, null-terminated), 43 bytes with no padding. Return 0, or the DOS error code with `errno` set as in `docs/spec/io.md` (`ENOENT` when nothing matches). The disk transfer area is restored to its previous address before returning.

## `int int86(int intno, union REGS *in, union REGS *out)`

Loads `AX`, `BX`, `CX`, `DX`, `SI` and `DI` from `*in`, executes `INT intno`, and stores those registers and the carry flag (`cflag`, non-zero when set) in `*out`; returns `AX`. `in` and `out` may be the same. `DS`, `ES`, `BP` and the direction flag are as the caller had them during the interrupt and are restored after it.

## `struct SREGS`

Four `unsigned` members, `es`, `cs`, `ss` and `ds`, at offsets 0, 2, 4 and 6.

## `int int86x(int intno, union REGS *in, union REGS *out, struct SREGS *segs)`

As `int86`, except that `DS` and `ES` are loaded from `segs->ds` and `segs->es` for the interrupt, and their values when it returns are stored in `segs->ds` and `segs->es`; `segs->cs` and `segs->ss` are not used or changed. `DS`, `ES` and `BP` are restored afterwards. `in`, `out` and `segs` may overlap only as `in == out`.

## `int intdos(union REGS *in, union REGS *out)`, `int intdosx(union REGS *in, union REGS *out, struct SREGS *segs)`

`int86(0x21, in, out)` and `int86x(0x21, in, out, segs)`. Unlike Microsoft's, they do not set `_doserrno`, which libc16 does not have; the caller reads `out->x.cflag` and `out->x.ax`.

## `void segread(struct SREGS *segs)`

Stores the current `ES`, `CS`, `SS` and `DS` in `*segs`.

## `int bdos(int dosfn, unsigned dosdx, unsigned dosal)`

Executes INT 21h with `AH` = the low byte of `dosfn`, `DX` = `dosdx` and `AL` = the low byte of `dosal`, and returns `AX`. For the DOS functions that take only those registers, such as 02h (display character), 09h (display string), 0Eh (select drive) and 4Ch (terminate). `SI`, `DI`, `BP` and `DS` are preserved.

## `FP_SEG(fp)`, `FP_OFF(fp)`

Macros. `fp` is an lvalue of a far pointer type. `FP_SEG(fp)` is an lvalue of type `unsigned` designating the segment word of `fp`, and `FP_OFF(fp)` the offset word, so `FP_SEG(p) = seg; FP_OFF(p) = off;` makes `p` point at `seg:off`. Each evaluates `fp` once.

## `int kbhit(void)`

`<conio.h>`. Returns non-zero if a character is waiting at standard input, and 0 if not, without reading it (INT 21h function 0Bh). As with that function, DOS checks for Ctrl+C (or Ctrl+Break) first.

## `char *getenv(const char *name)`

Searches the program's environment (the segment at PSP offset 2Ch: `NAME=value` strings, each null-terminated, ending with an empty string) for an entry whose name equals `name` exactly, and returns its value, or a null pointer. The environment is outside `DGROUP`, so the value is copied to a static buffer of 256 bytes, which the next call overwrites; a value that does not fit is treated as absent. The program must not modify the result.

## `char *_fullpath(char *buf, const char *path, size_t size)`

Stores in `buf` the absolute path of `path`: an upper-case drive letter, `:\`, and the directories and name, with `/` read as `\`, `.` and `..` components resolved, repeated separators collapsed, and no trailing `\` except for a root. A relative `path` (with or without a drive) starts from that drive's current directory. Returns `buf`, or a null pointer: `ERANGE` when the result and its terminator need more than `size` bytes, `ENOENT` for an invalid drive, `EINVAL` for an empty `path`. Unlike Microsoft's, a null `buf` is not supported (`EINVAL`). The case of the input is kept; the path need not exist.

## Constraints

All are `__cdecl`. 8086 instructions only.
