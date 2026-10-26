# Spec: startup and argv

**Members:** `cstart` (core startup), `argv` (hook), `args` (command-tail parser)  
**Minimum DOS:** 2.0  
**Tests:** `tests/startup` (run by `tests/run-dos`, `tests/check-members`, `tests/startup/check-memory`), `tests/check-abi`, `tests/check-selective`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- DOS `.EXE` load state and INT 21h functions 4Ah (resize memory block), 4Ch (terminate with return code) and 62h (get PSP, DOS 3.0, used only by tests): Ralf Brown's Interrupt List; the MS-DOS Programmer's Reference.
- Splitting the command tail: Microsoft's documentation of how Microsoft C parses command-line arguments.
- Symbols the compiler references and the register convention of `main`: call-site experiments in `tests/abi` (`docs/ABI.md`).
- `_edata` and `_end`: defined by wlink, observed by linking a module that references them and reading the map.
- C standard (C99 5.1.2.2.1, "Program startup", and 5.1.2.2.3, "Program termination"): `argv[argc]` is a null pointer, the strings are modifiable, `argv[0]` is `""` when the program name is not available, and returning from the initial call to `main` is equivalent to calling `exit` with its value.

## Core startup (`cstart`)

**Linked by:** the compiler's reference to `_cstart_` from the module defining `main`. Nothing else may be linked by a program whose `main` is `int main(void)`.

**Defines:** `_cstart_` (the program entry point, set with the `end` directive), `__psp`, `___libc16_argc`, `___libc16_argv`.

**State on entry (DOS):** `DS` = `ES` = PSP segment; `CS:IP` and `SS:SP` from the `.EXE` header, with `SS:SP` at the top of the `STACK` segment as the linker sized it.

**Must, in an order that is safe on an 8088:**

1. Set `DS` to `DGROUP`.
2. Set `SS` to `DGROUP` and `SP` to the same linear address as on entry, expressed relative to `DGROUP`, so that `option stack=n` keeps working. An `SP` that wraps to 0 means the stack ends at the top of a full 64 KiB `DGROUP`. Interrupts must be disabled while `SS` and `SP` disagree.
3. Resize the program's memory block (INT 21h function 4Ah, `ES` = PSP) to end at the top of the stack: (`DGROUP` − PSP) paragraphs plus the stack top rounded up to a paragraph. `tests/startup/check-memory` checks this exactly.
4. Zero every byte from `_edata` to `_end` with the direction flag clear. `_psp` (C name; symbol `__psp`, an `unsigned`) must hold the PSP segment once BSS is zeroed.
5. Call each near pointer in the init table, in order.
6. Call `main_` with `AX` = `__libc16_argc` and `DX` = `__libc16_argv` (both 0 unless an init routine set them).
7. Terminate with INT 21h function 4Ch with `AL` = the low byte of `main`'s return value.

**Segments:** declares `dosseg` ordering; `DGROUP` groups `CONST`, `CONST2`, `_DATA` (class `DATA`), `LIBC16_INIT_BEGIN`, `LIBC16_INIT`, `LIBC16_INIT_END` (class `DATA`, in that order), `_BSS` (class `BSS`) and `STACK` (class `STACK`, combine type stack, default 2048 bytes).

**Init table:** the entries are the near pointers in `LIBC16_INIT`, between a label at the start of `LIBC16_INIT_BEGIN` and one at the start of `LIBC16_INIT_END`. Init routines take no arguments and preserve `SI`, `DI`, `BP`, `DS` and `SS`.

**8086 only.** No instruction from the 80186 or later (`scripts/check-8086`).

## argv hook (`argv`)

**Linked by:** the compiler's reference to `__argc` from a `main` that takes parameters.

**Defines:** `__argc`, a label with no storage. **References:** `__libc16_args_`.

**Must:** add `__libc16_args_` to the init table, declaring the three init segments in order.

## Command-tail parser (`args`)

**Defines:** `void __libc16_args(void)` (default convention; symbol `__libc16_args_`).

**Must:** read the command tail from the PSP (length byte at offset 80h, text from 81h, ended by the length or by a carriage return, at most 127 bytes), split it into arguments, and set `__libc16_argc` and `__libc16_argv`:

- arguments are separated by runs of spaces and tabs; leading and trailing ones produce no argument;
- a double quote is removed and toggles quoting; inside quotes, spaces and tabs belong to the argument; `""` produces an empty argument;
- 2n backslashes followed by a double quote give n backslashes, and the quote toggles quoting; 2n + 1 backslashes followed by a double quote give n backslashes and a literal double quote; backslashes not followed by a double quote are literal;
- `argv[0]` is `""`, and `argv[argc]` is a null pointer;
- the strings are in `DGROUP` (near pointers) and modifiable; the arrays hold the most arguments a 127-byte tail can produce (64), with no heap.

The expected outputs in `tests/startup/*.expect` are part of this spec.
