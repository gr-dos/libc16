# Spec: running programs

**Headers:** `<process.h>`, `<stdlib.h>` (`system`)  
**Members:** `system`, `spawnl`, `spawnv`, `spawnve`, `spawnvp`, `spawnvpe`, `getpid`, each its own member; private members `spawnx` (the shared core: the mode, the command tail, the environment block, the extensions), `spawnp` (the `PATH` search) and `dosexec` (INT 21h 4B00h and 4Dh, in assembly); in each of the small, compact and large libraries (decision 11)  
**Tests:** `tests/process` (run by `tests/run-dos`, in every model)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation. `dosexec` is written in assembly: under DOS 2.x, INT 21h 4B00h returns with every register changed, `SS` and `SP` included, which C cannot express.

## Sources

- C89 7.10.4.5 (`system`).
- The interfaces of Microsoft C 5.1's `system`, `spawnl`, `spawnv`, `spawnve`, `spawnvp`, `spawnvpe` and `getpid`, the `P_WAIT`, `P_NOWAIT` and `P_OVERLAY` values, and their documented behaviour (the `.COM` then `.EXE` extension rule; a null `envp` inherits; the caller flushes its streams): the Microsoft C 5.1 run-time library reference.
- INT 21h functions 4B00h (load and execute: the parameter block, the environment block, the command tail), 4Dh (get return code: `AL` the code, `AH` how the child ended), 48h (allocate memory), 49h (free memory) and 43h (get attributes), all DOS 2.0; the environment and the program name DOS 3.0 adds after it; PSP offsets 2Ch, 5Ch, 6Ch and 80h; the registers 4B00h destroys under DOS 2.x: Ralf Brown's Interrupt List; MS-DOS 2.0 `SYSCALL.DOC`.
- `docs/spec/startup.md` ("Command-tail parser"), for the quoting the command tail uses, so that a libc16 child sees the `argv` its parent passed.
- gr-dos/perl16's `docs/M3-RUNTIME-AUDIT.md`, section 6.6, for the proposed contract.
- Our own experiments under DOSBox-X (0.09, `tests/run-dos`):
  - 4B00h of a name without an extension fails with error 2: DOS adds none.
  - 4B00h of a missing file, or of a file in a missing directory, fails with error 2.
  - With the environment segment 0 the child gets a copy of the parent's environment; with a segment, a copy of that block, up to its first pair of zero bytes, then the word 1 and the child's full path (`C:\CHILD.EXE`), also for a relative name. A block of just two zero bytes gives the child an empty environment.
  - Under DOSBox-X, `COMSPEC` is `Z:\COMMAND.COM`, and `Z:\COMMAND.COM /C cmd` runs `cmd` but returns 1 (4Dh) whenever it ran a program, whatever that program's exit code, and 0 otherwise. 4300h of `Z:\COMMAND.COM` succeeds.

## Memory

DOS loads a child into the largest free block. The startup shrinks the program's memory block to end at the top of its stack (`docs/spec/startup.md`), and the heap grows the block as it needs more but never shrinks it, not even when everything is freed (`docs/spec/heap.md`). So a child gets all the memory above the highest point the heap has reached. These functions leave the program's block and the heap as they are: they call no heap function, and a block they take from DOS (the environment of the `e` forms) is freed before they return.

## Common behaviour of the spawn functions

`int spawnv(int mode, const char *path, char * const *argv)`, `spawnve(int mode, const char *path, char * const *argv, char * const *envp)`, `spawnvp(int mode, const char *path, char * const *argv)`, `spawnvpe(int mode, const char *path, char * const *argv, char * const *envp)` and `spawnl(int mode, const char *path, const char *arg0, ...)` run the program `path` with arguments and wait for it to end.

- **`mode`:** `P_WAIT` (0) runs the child and waits for it. `P_NOWAIT` (1), `P_OVERLAY` (2) and any other value fail with `EINVAL`; nothing is run.
- **Arguments:** `argv` is a null-terminated array; `argv[0]` is the child's name by convention and is not passed (DOS gives the child its path itself). `spawnl` takes them as its arguments `arg0`, ... ended by a null pointer, and behaves as `spawnv` with them. A null `argv`, or a null `argv[0]`, fails with `EINVAL`.
- **Command tail:** built from `argv[1]` onward, each argument preceded by one space; no arguments gives an empty tail. An argument that is not empty and contains no space, tab or double quote is copied as it is. Any other argument is written in double quotes, so that the startup's parser gives it back unchanged: inside the quotes, `n` backslashes followed by a double quote are written as `2n + 1` backslashes and the quote, `n` backslashes at the end of the argument as `2n` backslashes, and any other character as itself. The tail is at most 126 bytes, not counting its length byte or the carriage return after it; a longer one fails with `E2BIG` and nothing is run.
- **FCBs:** the parameter block's two FCB pointers point to a blank FCB (drive 0, then eleven spaces, then zeros), so the child's FCBs at PSP offsets 5Ch and 6Ch are blank.
- **Environment:** `spawnv`, `spawnvp` and `spawnl` pass the parent's environment (segment 0). `spawnve` and `spawnvpe` build a block from `envp`: each string in order with its terminating zero, then one more zero; an `envp` with no strings gives two zero bytes. The block is built from `envp` alone, with no use of `environ` (libc16 has none) or `getenv`. A block of more than 32767 bytes fails with `E2BIG`. A null `envp` passes the parent's environment. The block is in memory taken with INT 21h 48h, freed with 49h after the child ends or the call fails; if 48h fails, the call fails with `ENOMEM`.
- **Extensions:** if the last component of the name (after the last `\`, `/` or `:`) contains no `.`, the name with `.COM` is tried, then with `.EXE`. Otherwise the name is tried as it is.
- **`PATH` (`spawnvp`, `spawnvpe` only):** if `path` contains `\`, `/` or `:`, only `path` is tried. Otherwise `path` is tried first (in the current directory), then each element of the parent's `PATH`, in order, with `\` between the element and `path` unless the element ends in `\`, `/` or `:`. Elements are separated by `;`; empty ones are skipped. The search reads `PATH` with `getenv` (`docs/spec/dos.md`), so it overwrites `getenv`'s buffer, and uses the parent's `PATH` even when `envp` has one. A candidate (with its extension) that does not fit in 128 bytes, terminator included, is skipped.
- **Trying a name:** each candidate, with each extension, is run with INT 21h 4B00h. Errors 2 (file not found), 3 (path not found) and 15 (invalid drive) move on to the next one; any other error ends the search, failing with its `errno` as in `docs/spec/io.md` (8 is `ENOMEM`, 11 `ENOEXEC`, 5 `EACCES`). When every candidate fails with 2, 3 or 15, the call fails with `ENOENT`.
- **Result:** the child's exit code: `AL` of INT 21h 4Dh, 0 to 255 (`AH`, how the child ended, is not returned). On failure, -1 with `errno` set; `errno` is unchanged on success.
- **Across the call:** the child inherits the parent's open handles, as DOS does. The functions do not flush stdio streams; a caller that wants its buffered output to come before the child's flushes it first (`fflush`). `dosexec` saves what it needs, `SS` and `SP` included, where it can find them again with every register but `CS` and `IP` changed, and returns with `DS`, `SS`, `SP`, `BP`, `SI` and `DI` as they were (`docs/ABI.md`) and the direction flag clear.
- All are `__cdecl` (`docs/ABI.md`); the pointers are near in the small model and far in the compact and large models. `spawnv`, `spawnve` and `spawnl` link no heap, stdio or `getenv`; the `p` forms link `getenv` and so the startup.

## `int system(const char *cmd)`

Runs the command processor named by `COMSPEC` (from `getenv`), or `\COMMAND.COM` (the root of the current drive) if `COMSPEC` is not set, with the parent's environment and the command tail ` /C ` followed by `cmd` exactly as it is (no quoting), and returns the command processor's exit code (4Dh's `AL`). `cmd` longer than 122 bytes fails with `E2BIG`, as the tail would pass 126. If the command processor cannot be run, `system` returns -1 with `errno` set as for the spawn functions (`ENOENT` for a missing one). No extension is added and no `PATH` search is made.

`system(NULL)` runs nothing and returns 1 if the command processor exists (INT 21h 4300h succeeds and it is not a directory), else 0.

The exit code is the command processor's, not necessarily the command's: DOSBox-X's `COMMAND.COM` returns 1 whenever it ran a program and 0 otherwise (see "Sources"). C89 leaves the value implementation-defined.

`system` links `getenv` and so the startup, and no heap or stdio.

## `int getpid(void)`

Returns the program's PSP segment (`_psp`). It is unique among the programs running at the same time: a child's PSP is in memory the parent's block does not contain. Links the startup.

## `<process.h>`

Declares the spawn functions, `system`, `getpid`, and `P_WAIT` (0), `P_NOWAIT` (1) and `P_OVERLAY` (2). `<stdlib.h>` declares `system` too.

## Tests

`tests/process`, in every model, with `CHILD.EXE` (`tests/process/child.c`), a libc16 program that prints, one per line: `name[` its full path (from after its environment) `]`; `arg[` each argument `]`; with `-e` as its first argument, `env[` each environment string `]`; with `-g NAME`, `val[` the value of `NAME` `]`, or `val(null)`; with `-f`, `fcb[` its first FCB's drive (a digit) and the 11 name bytes `]`. It exits with the value of its last argument if that starts with a digit, else 0.

- `process`: exit codes (0, 7, 255); arguments with spaces, tabs, double quotes, backslashes and empty ones, seen exactly by the child; the 126-byte limit with and without quoting (`E2BIG`, nothing run); blank FCBs; the extension rule (`CHILD` runs `CHILD.EXE`, `KID` finds `KID.COM` before `KID.EXE`); the `PATH` search, with missing and empty elements and an element ending in `\`, the current directory first, no search for a name with a `\` or for `spawnv`, and the parent's `PATH` for `spawnvpe`; an `envp` the child sees, an empty one, a null one (inherits) and one over 32767 bytes (`E2BIG`); inheritance of a variable the cases set; `P_NOWAIT`, `P_OVERLAY` and 99 (`EINVAL`); a null `argv`; missing programs (`ENOENT`); `errno` unchanged on success; the program's memory block and the largest free block the same after each call; `getpid` equal to `_psp` and to INT 21h 62h.
- `nopath`: `spawnl`, `spawnv` and `spawnve` alone, so that `tests/check-members` checks they link no `getenv`, heap or stdio.
- `system`, run four times by the cases: with `COMSPEC` set to `C:\CHILD.EXE` (the exit code passes through; the tail is ` /C ` and the command; `system(NULL)` is 1; 122 and 123 bytes); with `COMSPEC` unset (`system(NULL)` is 0 and `system` fails with `ENOENT` until the test copies `CHILD.EXE` to `\COMMAND.COM`, then runs it); with `COMSPEC` naming a directory (`system(NULL)` is 0 and `system` fails); and with DOSBox-X's `Z:\COMMAND.COM` (a command with redirection runs, and its output file has the child's output).
- 8086 instructions only (`tests/check-8086`).
