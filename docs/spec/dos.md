# Spec: DOS interface, environment and paths

**Headers:** `<dos.h>`, `<conio.h>`, `<stdlib.h>`  
**Members:** one per function  
**Tests:** `tests/dos`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- INT 21h functions 02h (character output), 06h (direct console I/O), 08h (character input without echo), 0Bh (check input status), 19h (get drive), 1Ah (set disk transfer area), 2Fh (get disk transfer area), 47h (get current directory), 4Eh (find first) and 4Fh (find next), all DOS 2.0; the environment segment at PSP offset 2Ch: Ralf Brown's Interrupt List; MS-DOS 2.0 `SYSCALL.DOC`.
- The interfaces of Microsoft C 5.1's `_dos_open`, `_dos_creat`, `_dos_close`, `_dos_read`, `_dos_write`, `_dos_getfileattr`, `_dos_setfileattr`, `_dos_getvect`, `_dos_setvect`, `_dos_findfirst`, `_dos_findnext`, `int86`, `int86x`, `intdos`, `intdosx`, `segread`, `bdos`, `kbhit`, `getch`, `putch`, `getenv` and `_fullpath`, the `FP_SEG` and `FP_OFF` macros, and the layouts of `struct find_t`, `union REGS` and `struct SREGS`: the Microsoft C 5.1 run-time library reference. GR-DOS's ATTRIB, MEM and FDISK, from the MS-DOS 4.00 source, are the call sites (`docs/CONSUMERS.md`).
- C89 7.10.4.4 (`getenv`).
- Open Watcom C library documentation for `_harderr` with a handler that returns its action, and for `MK_FP`; Ralf Brown's Interrupt List for INT 24h (registers on entry, the stack it runs on, the functions a handler may call, the return codes in `AL`).

## `unsigned _dos_findfirst(const char *path, unsigned attr, struct find_t *f)`, `unsigned _dos_findnext(struct find_t *f)`

Find the first file matching `path` (which may contain wildcards) with attributes `attr` (`_A_*`, as INT 21h 4Eh interprets them), or the next one of the same search. `struct find_t` is DOS's search record: 21 reserved bytes, `attrib`, `wr_time`, `wr_date`, `size`, and `name` (the 8.3 name, null-terminated), 43 bytes with no padding. Return 0, or the DOS error code with `errno` set as in `docs/spec/io.md` (`ENOENT` when nothing matches). The disk transfer area is restored to its previous address before returning.

## `_dos_open`, `_dos_creat`, `_dos_close`, `_dos_read`, `_dos_write`, `_dos_getfileattr`, `_dos_setfileattr`

Microsoft C's direct DOS file calls, each one INT 21h function. Each returns 0, or the DOS error code with `errno` set as in `docs/spec/io.md`; output parameters are stored only on success. They use no handle table: the handles are DOS's, untranslated (binary).

- **`unsigned _dos_open(const char *path, unsigned mode, int *handle)`:** function 3Dh with `AL` = the low byte of `mode` (`O_RDONLY`, `O_WRONLY` or `O_RDWR`, with DOS's sharing and inheritance bits if wanted); stores the handle.
- **`unsigned _dos_creat(const char *path, unsigned attr, int *handle)`:** function 3Ch: creates or truncates the file with attributes `attr` (`_A_NORMAL`, `_A_RDONLY`, `_A_HIDDEN`, `_A_SYSTEM`, `_A_ARCH`); stores the handle.
- **`unsigned _dos_close(int handle)`:** function 3Eh.
- **`unsigned _dos_read(int handle, void __far *buf, unsigned count, unsigned *nread)`:** function 3Fh into `buf`, which is far in every model; stores the number of bytes read (0 at end of file).
- **`unsigned _dos_write(int handle, const void __far *buf, unsigned count, unsigned *nwritten)`:** function 40h from `buf`, far in every model; stores the number of bytes written, which may be less than `count` when the disk is full. A `count` of 0 truncates the file at its pointer, as DOS does.
- **`unsigned _dos_getfileattr(const char *path, unsigned *attr)`:** function 4300h; stores the attributes.
- **`unsigned _dos_setfileattr(const char *path, unsigned attr)`:** function 4301h.

## `_dos_getvect`, `_dos_setvect`

- **`void (__interrupt __far *_dos_getvect(unsigned intno))()`:** the interrupt vector `intno` (low byte), from INT 21h function 35h.
- **`void _dos_setvect(unsigned intno, void (__interrupt __far *handler)())`:** sets the vector with INT 21h function 25h.

## `void _harderr(int (__cdecl __far *handler)(unsigned deverror, unsigned errcode, unsigned __far *devhdr))`

Installs `handler` as the program's critical-error (INT 24h) handler, through INT 21h function 25h; DOS restores its own handler when the program ends. When DOS reports a critical error, `handler` is called with `deverror` = `AX` (bit 15 set for a device other than a disk; for a disk, `AL` is the drive and `AH` the error flags), `errcode` = `DI` (the error code in its low byte) and `devhdr` = `BP:SI` (the device header), as DOS passes them to INT 24h. Its return value is DOS's action, in `AL`: `_HARDERR_IGNORE` (0), `_HARDERR_RETRY` (1), `_HARDERR_ABORT` (2, ends the program) or `_HARDERR_FAIL` (3, DOS 3.0 and later: the DOS call fails).

This is Open Watcom's documented form, where the handler returns its action; Microsoft C 5.1's handler ended with `_hardresume` or `_hardretn`, which libc16 does not provide. The handler type is `__cdecl` (decision 2), so a default-convention handler does not compile.

`handler` runs with `DS` and `SS` set to `DGROUP`, on a stack of 512 bytes that belongs to `_harderr` (DOS calls INT 24h on its own stack), with the direction flag clear. As DOS requires, it may call only INT 21h functions 01h to 0Ch, 30h and 59h, which rules out every libc16 file and stream function. Every register but `AX` is restored before returning to DOS.

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

## `FP_SEG(fp)`, `FP_OFF(fp)`, `MK_FP(seg, off)`

Macros. `MK_FP(seg, off)` is a `void __far *` pointing at `seg:off`, as in Open Watcom's documentation. For the others, `fp` is an lvalue of a far pointer type. `FP_SEG(fp)` is an lvalue of type `unsigned` designating the segment word of `fp`, and `FP_OFF(fp)` the offset word, so `FP_SEG(p) = seg; FP_OFF(p) = off;` makes `p` point at `seg:off`. Each evaluates `fp` once.

## `int kbhit(void)`

`<conio.h>`. Returns non-zero if a character is waiting at standard input, and 0 if not, without reading it (INT 21h function 0Bh). As with that function, DOS checks for Ctrl+C (or Ctrl+Break) first.

## `int getch(void)`

`<conio.h>`. Reads one character from standard input without echoing it and returns it, 0 to 255 (INT 21h function 08h). It waits for a character if none is waiting. A key with no character (a function or cursor key) is read as two characters, 0 and then its scan code, as DOS gives them. As with function 08h, DOS checks for Ctrl+C (or Ctrl+Break). Unlike Microsoft's, there is no `ungetch`.

## `int putch(int c)`

`<conio.h>`. Writes the character `c` (its low byte) to standard output and returns it, as an `unsigned char` (0 to 255). It uses INT 21h function 06h, which does not check for Ctrl+C. Function 06h takes the byte 0FFh as a request for input, so 0FFh is written with function 02h instead.

## `char *getenv(const char *name)`

Searches the program's environment (the segment at PSP offset 2Ch: `NAME=value` strings, each null-terminated, ending with an empty string) for an entry whose name equals `name` exactly, and returns its value, or a null pointer. The environment is outside `DGROUP`, so the value is copied to a static buffer of 256 bytes, which the next call overwrites; a value that does not fit is treated as absent. The program must not modify the result.

## `char *_fullpath(char *buf, const char *path, size_t size)`

Stores in `buf` the absolute path of `path`: an upper-case drive letter, `:\`, and the directories and name, with `/` read as `\`, `.` and `..` components resolved, repeated separators collapsed, and no trailing `\` except for a root. A relative `path` (with or without a drive) starts from that drive's current directory. Returns `buf`, or a null pointer: `ERANGE` when the result and its terminator need more than `size` bytes, `ENOENT` for an invalid drive, `EINVAL` for an empty `path`. Unlike Microsoft's, a null `buf` is not supported (`EINVAL`). The case of the input is kept; the path need not exist.

## Constraints

All are `__cdecl`. 8086 instructions only.
