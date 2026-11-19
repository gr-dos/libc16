# Spec: startup, argv and exit

**Members:** `cstart` (core startup), `argv` (hook), `args` (command-tail parser), `exit`, `atexit`, in each of the small, compact and large libraries (decision 11)  
**Minimum DOS:** 2.0  
**Tests:** `tests/startup` (run by `tests/run-dos`, `tests/check-members`, `tests/startup/check-memory`, `tests/startup/check-null`), `tests/check-abi`, `tests/check-selective`; `atexit` in `tests/stdlib`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- DOS `.EXE` load state and INT 21h functions 4Ah (resize memory block), 4Ch (terminate with return code) and 62h (get PSP, DOS 3.0, used only by tests): Ralf Brown's Interrupt List; the MS-DOS Programmer's Reference.
- Splitting the command tail: Microsoft's documentation of how Microsoft C parses command-line arguments.
- Symbols the compiler references and the register convention of `main`: call-site experiments in `tests/abi` (`docs/ABI.md`).
- `_edata` and `_end`: defined by wlink, observed by linking a module that references them and reading the map.
- C standard (C99 5.1.2.2.1, "Program startup", and 5.1.2.2.3, "Program termination"): `argv[argc]` is a null pointer, the strings are modifiable, `argv[0]` is `""` when the program name is not available, and returning from the initial call to `main` is equivalent to calling `exit` with its value. C99 7.20.4.3 (`exit`); C89 7.10.4.2 (`atexit`) and 7.10.4.3 (`exit`).
- The termination table: DESIGN.md decision 8, which reserves the init table's mechanism for termination routines such as flushing buffered output.
- A program with its own entry point: GR-DOS's ATTRIB, whose `ATTRIBA.ASM` (MS-DOS 4.00 source) sets up `DGROUP` and calls C itself.

## Memory models

Every member is built for the small, compact and large models from one source (`crt/model.inc`; `docs/ABI.md`, "Memory models"). What differs:

| | small | compact | large |
|---|---|---|---|
| Call to `main_`, init routines, `__libc16_args_` | near | near | far |
| `argv` passed to `main_` | `DX` | `CX:BX` (segment in `CX`) | `CX:BX` |
| `___libc16_argv` | 2 bytes, near | 4 bytes, far (offset first) | 4 bytes, far |
| Init and termination table entry | 2 bytes, near | 2 bytes, near | 4 bytes, far (offset first) |
| `atexit` function pointer | near | near | far |
| `DS` after a call to C code | `DGROUP` | any | any |
| `exit`'s argument, on entry | `[SP+2]` | `[SP+2]` | `[SP+4]` |

Everything else, including `DGROUP`'s layout and the memory block left to DOS, is the same in every model, and the small model's code is unchanged. With far data the startup may not assume `DS` is `DGROUP` after calling an init routine or any other C code, and sets it to `DGROUP` again before calling `main_`.

## Core startup (`cstart`)

**Linked by:** the compiler's reference to `_cstart_` from the module defining `main`. Nothing else may be linked by a program whose `main` is `int main(void)`.

**Defines:** `_cstart_` (the program entry point, set with the `end` directive), `__psp`, `___libc16_argc`, `___libc16_argv`, `___libc16_stack_top`.

**State on entry (DOS):** `DS` = `ES` = PSP segment; `CS:IP` and `SS:SP` from the `.EXE` header, with `SS:SP` at the top of the `STACK` segment as the linker sized it.

**Must, in an order that is safe on an 8088:**

1. Set `DS` to `DGROUP`.
2. Set `SS` to `DGROUP` and `SP` to the same linear address as on entry, expressed relative to `DGROUP`, so that `option stack=n` keeps working. An `SP` that wraps to 0 means the stack ends at the top of a full 64 KiB `DGROUP`. Interrupts must be disabled while `SS` and `SP` disagree.
3. Resize the program's memory block (INT 21h function 4Ah, `ES` = PSP) to end at the top of the stack: (`DGROUP` − PSP) paragraphs plus the stack top rounded up to a paragraph. `tests/startup/check-memory` checks this exactly.
4. Zero every byte from `_edata` to `_end` with the direction flag clear. `_psp` (C name; symbol `__psp`, an `unsigned`) must hold the PSP segment once BSS is zeroed.
5. Call each pointer in the init table, in order (near, or far in the large model).
6. Call `main_` with `DS` = `DGROUP`, `AX` = `__libc16_argc` and `argv` = `__libc16_argv` (`DX` in the small model, `CX:BX` with far data; both 0 unless an init routine set them).
7. When `main_` returns, call each pointer in the termination table, as `exit` does ("Termination"), then terminate with INT 21h function 4Ch with `AL` = the low byte of `main`'s return value. The table's routines may change any register `main`'s value is in; the value is kept across them.

**Null pointer:** no object of the program may be at offset 0 of `DGROUP`, because a pointer to it would be a null pointer (C99 6.3.2.3). The startup reserves the start of `DGROUP`, before `CONST`, with at least 2 bytes that hold no C object, whatever the size of the code and the order of the other modules.

The reserved bytes are 0 and start on a paragraph, so `DGROUP`'s frame, the segment the startup loads into `DS`, is their paragraph and they are at `DS:0000`, whatever the size of the code and other segments before `DGROUP`. These are two separate properties:

- *No valid object at offset 0* is what C needs, and holds as long as something non-empty comes first in `DGROUP`.
- *What is at `DS:0000`* is a property of the layout. When the reserved bytes were only word aligned, `DGROUP`'s frame was the paragraph containing them, and the bytes from `DS:0000` up to them, as many as 14, could be the last bytes of the code or of a far data segment. A near write through a null pointer then changed code. With the bytes on a paragraph it changes the reserved bytes instead.

Reading or writing through a null pointer is still undefined behaviour, and libc16 promises nothing about it: not that `DS:0000` reads as 0 when the program runs (an earlier null write may have changed it), nor that such a write is detected. The alignment costs at most 15 bytes of padding between the code and `DGROUP` in the `.EXE`, and at most one paragraph of memory.

**Null function pointer:** in the small and compact models a function pointer is an offset in the code segment, and without a reserve the first function of the first module linked is at offset 0, so a pointer to it compares equal to a null function pointer (C99 6.3.2.3), and to `SIG_DFL` (`docs/spec/signal.md`). The startup reserves offsets 0 and 1 of the code segment with 2 bytes that hold no function, whatever the order of the modules, so no function is at offset 0 or 1 (`SIG_IGN` is 1). The bytes are 0 and are never executed. The reserve costs 2 bytes in the `.EXE` and in memory. In the large model a function pointer has a segment that is never 0 in a loaded program, and the startup reserves nothing.

The reserve is a segment of class `BEGCODE`, which `dosseg` ordering puts before every `CODE` segment whatever the order of the modules, paragraph aligned, so that the code segment's frame (`CS`) is its paragraph and `_TEXT`, byte aligned as wcc emits it, follows at offset 2. wlink keeps it: it discards no segment unless asked to (`option eliminate` discards only unreferenced code, and the startup references the reserve).

**Segments:** declares `dosseg` ordering; in the small and compact models, `LIBC16_CODE0` (class `BEGCODE`, paragraph aligned, 2 bytes of 0: the code reserve, see "Null function pointer") before `_TEXT`; `DGROUP` groups `LIBC16_NULL` (class `BEGDATA`, paragraph aligned, 2 bytes of 0: the reserved start, which `dosseg` puts first in `DGROUP` whatever the order of the modules), `CONST`, `CONST2`, `_DATA` (class `DATA`), `LIBC16_INIT_BEGIN`, `LIBC16_INIT`, `LIBC16_INIT_END` (class `DATA`, in that order), `LIBC16_EXIT_BEGIN`, `LIBC16_EXIT`, `LIBC16_EXIT_END` (class `DATA`, in that order, after the init table), `_BSS` (class `BSS`), `STACK` (class `STACK`, combine type stack, default 2048 bytes) and `LIBC16_STACK_END` (class `STACK`, byte aligned, empty).

**Top of the stack:** `__libc16_stack_top` (C name, declared `extern char __cdecl __libc16_stack_top[]`; symbol `___libc16_stack_top`) is a label with no storage at the start of `LIBC16_STACK_END`, so its offset in `DGROUP` is the top of the stack for any `option stack=n`: the address step 2 sets `SP` to, and where the program's memory block ends, rounded up to a paragraph (step 3). The heap starts there (`docs/spec/heap.md`). It adds no bytes to a program, so programs that do not use it are unchanged. `tests/startup/check-memory` checks it against the end of `DGROUP` in the map.

**Init table:** the entries are the pointers in `LIBC16_INIT` (near, or far in the large model; see "Memory models"), between a label at the start of `LIBC16_INIT_BEGIN` and one at the start of `LIBC16_INIT_END`. Init routines take no arguments and preserve `SI`, `DI`, `BP` and `SS`, and `DS` in the small model.

**Termination table:** the same for termination, in `LIBC16_EXIT_BEGIN`, `LIBC16_EXIT` and `LIBC16_EXIT_END`: pointers of the same size, between a label at the start of `LIBC16_EXIT_BEGIN` and one at the start of `LIBC16_EXIT_END`, to routines with the init routines' contract. A member adds an entry by declaring the three init segments and then the three termination segments, in that order, in `DGROUP`, so that the termination table follows the init table whichever module the linker sees first. (Both walks use the labels, so the order of the two tables does not change what runs.) The table is walked by both the startup (step 7) and `exit`, so it does not matter which of them a program links first.

**8086 only.** No instruction from the 80186 or later (`scripts/check-8086`).

## argv hook (`argv`)

**Linked by:** the compiler's reference to `__argc` from a `main` that takes parameters.

**Defines:** `__argc`, a label with no storage. **References:** `__libc16_args_`.

**Must:** add `__libc16_args_` to the init table (a near pointer, or a far one in the large model), declaring the three init segments in order.

## Command-tail parser (`args`)

**Defines:** `void __libc16_args(void)` (default convention; symbol `__libc16_args_`).

**Must:** read the command tail from the PSP (length byte at offset 80h, text from 81h, ended by the length or by a carriage return, at most 127 bytes), split it into arguments, and set `__libc16_argc` and `__libc16_argv`:

- arguments are separated by runs of spaces and tabs; leading and trailing ones produce no argument;
- a double quote is removed and toggles quoting; inside quotes, spaces and tabs belong to the argument; `""` produces an empty argument;
- 2n backslashes followed by a double quote give n backslashes, and the quote toggles quoting; 2n + 1 backslashes followed by a double quote give n backslashes and a literal double quote; backslashes not followed by a double quote are literal;
- `argv[0]` is `""`, and `argv[argc]` is a null pointer;
- the strings and the `argv` array are in `DGROUP` (pointers to them are near in the small model and far with far data) and modifiable; the arrays hold the most arguments a 127-byte tail can produce (64), with no heap.

The expected outputs in `tests/startup/*.expect` are part of this spec.

## Termination (`exit`)

**Header:** `<stdlib.h>`, `void exit(int status)` (`__cdecl`; symbol `_exit`).

**Must:** call each pointer in the termination table ("Core startup", "Termination table"), in table order, then terminate the program with INT 21h function 4Ch, `AL` = the low byte of `status`. It does not return. Each routine is called near, or far in the large model, with `SS` = `DGROUP`, the direction flag clear and, in the small model, `DS` = `DGROUP`: what C code expects in each model (`docs/ABI.md`), as for an init routine. Buffered stream output is written by a routine in the table, linked with `setvbuf` and `setbuf` (`docs/spec/stdio.md`, "Termination"); DOS closes the program's handles.

A routine may change `DS` in the compact and large models, as an init routine may, so `exit` and the startup do not read the table through `DS` after calling one without setting `DS` again.

**References** nothing else: a program that calls `exit` links only `exit` for it, with or without the core startup. An empty table, in a program that links no member that adds an entry, costs only the code that walks it, in `exit` and in the startup: no further member and no data.

**A routine that calls `exit`** (directly, or through an `atexit` function): C89 leaves this undefined. libc16's `exit` walks the table again from its start with the new `status`, so a routine may be called a second time; a routine in the table must allow that (`atexit`'s, below, does). The program then terminates with the new `status`.

## `int atexit(void (__cdecl *func)(void))`

**Header:** `<stdlib.h>` (`__cdecl`; symbol `_atexit`).

Registers `func` to be called, without arguments, when the program terminates through `exit` or by returning from `main` (C89 7.10.4.2), and returns 0. Up to 32 functions can be registered, the least C89 allows; once 32 are registered, `atexit` returns a nonzero value and registers nothing. The same function registered twice is called twice.

At termination the functions are called in the reverse order of their registration, each once. `func` is a `__cdecl` function (`docs/ABI.md`, "Callbacks"), called near in the small and compact models and far in the large model, as the termination table's routines are (`SS` = `DGROUP`, the direction flag clear, and `DS` = `DGROUP` in the small model). `atexit` adds one routine to the termination table, which calls the registered functions; a program that does not call `atexit` does not link that routine or the 32 entries.

- **A function is removed before it is called,** so that each is called at most once whatever the functions do: one that calls `exit` makes the walk start again (above), and the remaining functions are called then, each once, in the same order, before the program terminates with the new status.
- **A function that registers another** while the functions are being called: the new one is the most recent, so it is called next. If 32 are registered, `atexit` fails as at any other time.
- `abort` does not call them (C89 7.10.4.1; `docs/spec/stdlib.md`).
- The registered functions are in BSS, so a program with its own entry point (below) gets an empty list from zeroing BSS.

## Programs without the startup

A program may have its own entry point instead of `cstart`. It defines no `main`, so nothing references `_cstart_`. Before it calls any C code, its entry must:

- set `DS` and `SS` to `DGROUP`, with `SP` in `DGROUP` (in the small model near pointers reach the stack, and in every model `SS` is `DGROUP`);
- zero every byte from `_edata` to `_end`;
- clear the direction flag;

and it must terminate the program itself, or through `exit`. In the large model it calls C code far. It should also keep offset 0 of `DGROUP` free of C objects (see "Null pointer"), for example with a paragraph-aligned 2-byte segment of class `BEGDATA`. In the small and compact models it should likewise keep code offsets 0 and 1 free of functions whose address is taken (see "Null function pointer"), for example with a paragraph-aligned 2-byte segment of class `BEGCODE` and `dosseg` ordering, or by making its own entry code the first bytes of its first module: libc16 provides no reserve without the startup.

Such a program may use every member that references no symbol `cstart` defines (`__psp`, `___libc16_stack_top`, `___libc16_argc`, `___libc16_argv`) and is not an init routine: the string functions, `int86`, `int86x`, `intdos`, `intdosx`, `segread`, `bdos`, `kbhit`, `_dos_findfirst`, `_dos_findnext`, `_fullpath`, the low-level I/O functions of `docs/spec/io.md` (`isatty`, `dup` and `dup2` among them), the stdio functions (`fdopen` among them), `rmdir`, `libc16_ultoa`, `libc16_ltoa`, `strtoul`, `atoi`, `strftime`, `gmtime`, `rand`, `srand`, the `<ctype.h>` functions, the compiler support routines, `exit` and `atexit`. Its `exit` calls the `atexit` functions; it must terminate through `exit` for them to be called. The heap (`malloc`, `free`, `realloc`, `calloc`, `strdup`), `getenv` and `_psp` link `cstart`, and so do `tzset`, `localtime`, `time`, `stat` and `utime` through `getenv`, so a program with its own entry must not use them. `tests/startup/ownent` is such a program, and registers an `atexit` function; `tests/check-members` checks that it links no `cstart`.

## Tests of the code reserve

- `tests/startup/code0` is a program whose first function, the first code of the first module linked, would be at offset 0 without the reserve: it checks, through a `volatile` function pointer, that the function's pointer and `main`'s are not null, and in the small and compact models that their offsets are 2 or more.
- `tests/startup/check-null` also checks every test program's map in the small and compact models: the first code segment is `LIBC16_CODE0`, at least 2 bytes, at offset 0 of the code frame, and every other code segment with any bytes starts at offset 2 or later. In the large model it checks that there is no `LIBC16_CODE0`.
- `tests/signal`'s handlers (`docs/spec/signal.md`) keep passing, with no reserve of `signal`'s own.

## Tests of termination

- `tests/startup/fini` and `finiex` link two entries of their own in the termination table (`tests/startup/finitab.asm`): an assembly routine that, with far data, returns with `DS` = 0, then a C routine. `fini` returns 9 from `main` and `finiex` calls `exit(6)` from a nested function. Each checks that both routines run once, in table order, after `main`'s output, and that the exit status is `main`'s value or `exit`'s argument. `status` and `exitst` still link only `cstart`, and `cstart` and `exit`.
- `tests/stdlib/atexit` registers three functions and returns from `main`; `atexitx` registers them and calls `exit` from a nested function. Both check the reverse order and the exit status.
- `tests/stdlib/atexit32` registers one function 32 times, checks that a 33rd registration fails, and that the function is then called exactly 32 times.
- `tests/stdlib/atexitn` has a function that calls `exit(4)` and one that registers another function during termination; every function runs once, in the order above, and the status is 4.
- `tests/stdlib/abort` registers a function, which `abort` must not call.
- `tests/startup/ownent`, with its own entry point, registers a function and ends through `exit`.
