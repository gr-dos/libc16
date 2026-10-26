# libc16 ABI

**Toolchain:** open-watcom-v2 `2026-10-01-Build` (decision 1)  
**Target:** DOS real-mode `.EXE`, small memory model, 8086 instructions only

This is the contract between libc16 and the programs that link it. Every statement marked *tested* is checked by `make test` (`tests/check-abi`, `tests/run-dos`); the rest are compiler behaviour observed in the experiments below and still to be covered by tests.

## Consumer build

A consumer compiles with Open Watcom's **default register convention** (decision 2) and only libc16's headers:

```text
wcc -zl -zld -x -ms -0 -s -bt=dos -I$(LIBC16_ROOT)/include ...
wlink option nodefaultlibs format dos ... library $(LIBC16_ROOT)/LIBC16.LIB
```

| Flag | Why |
|---|---|
| `-x` | ignore `INCLUDE` and `WATCOM`, so no Open Watcom header is reachable |
| `-zl` | leave out default-library records |
| `-zld` | leave out file-dependency records |
| `-ms` | small model: one code segment, `DGROUP` for data and stack, near pointers |
| `-0` | 8086 instructions only |
| `-s` | no stack checks; libc16 does not provide the stack-check routine |
| `option nodefaultlibs` | resolve nothing from a default library; a missing symbol is a link error |

`-ecc` (making `__cdecl` the default) is **not** supported: the startup calls `main_`.

## Calling conventions

### Public library functions: `__cdecl`

Every public libc16 function is declared `__cdecl` in its header.

- **Symbol:** `_name` (*tested*).
- **Arguments:** pushed right to left; the caller pops them (`add sp,n`) (*tested*).
- **Return value:** `AX` (16-bit), `DX:AX` (32-bit).
- **Registers:** the callee may change `AX`, `BX`, `CX`, `DX` and `ES`, and must preserve `SI`, `DI`, `BP`, `DS` and `SS`. A default-convention caller saves `BX` and `CX` around a `__cdecl` call.
- **Direction flag:** clear on entry and on return.

### Callbacks

Library functions that take function pointers (`qsort`, `bsearch`, `atexit` and so on) take `__cdecl` function-pointer types. Passing a default-convention function is a compile error, E1071 (*tested*), so the application must declare its callback `__cdecl`.

### Variables

Global variables are `_name` whether or not they are declared `__cdecl` (*tested*). libc16 headers declare them `__cdecl` anyway, for uniformity.

### Application entry

- `main` is `main_`, with the register convention: `argc` in `AX`, `argv` in `DX`, result in `AX` (*tested* through `tests/run-dos`).
- The module that defines `main` references `_cstart_`, which links the core startup (*tested*).
- If `main` takes parameters, the module also references `__argc`, which links the argv members (*tested*; decision 8).

### Compiler support routines

Calls the compiler emits for operations it does not inline use Open Watcom's own symbols and register conventions (`docs/spec/arith.md`). They are private to libc16. The symbols are *tested* (`tests/check-abi`), and so is each register contract (`tests/arith`: results, and `regs`, which calls each helper with marker values in the preserved registers).

All are near calls (`call near ptr`). In the table, `a` is the left operand and `b` the right; a 32-bit pair `DX:AX` has the high word first, and a 64-bit quadruple `AX:BX:CX:DX` has `AX` as bits 63–48 and `DX` as bits 15–0. `[SI]` is an 8-byte operand at `DS:SI`, low word first, which the helper must leave unchanged.

| Symbol | Operation | In | Out | Changes |
|---|---|---|---|---|
| `__I4M`, `__U4M` | `long`, `unsigned long` `*` | `a` = `DX:AX`, `b` = `CX:BX` | `DX:AX` | `AX`, `BX`, `CX`, `DX` |
| `__I4D`, `__U4D` | `long`, `unsigned long` `/` and `%` | `a` = `DX:AX`, `b` = `CX:BX` | quotient `DX:AX`, remainder `CX:BX` | `AX`, `BX`, `CX`, `DX` |
| `__I8M`, `__U8M` | `long long`, `unsigned long long` `*` | `a` = `AX:BX:CX:DX`, `b` = `[SI]` | `AX:BX:CX:DX` | `AX`, `BX`, `CX`, `DX` |
| `__I8DQ`, `__U8DQ` | `long long`, `unsigned long long` `/` | `a` = `AX:BX:CX:DX`, `b` = `[SI]` | `AX:BX:CX:DX` | `AX`, `BX`, `CX`, `DX` |
| `__I8DR`, `__U8DR` | `long long`, `unsigned long long` `%` | `a` = `AX:BX:CX:DX`, `b` = `[SI]` | `AX:BX:CX:DX` | `AX`, `BX`, `CX`, `DX` |
| `__I8LS`, `__U8LS` | `long long`, `unsigned long long` `<<` | `a` = `AX:BX:CX:DX`, `n` = `SI` | `AX:BX:CX:DX` | `AX`, `BX`, `CX`, `DX` |
| `__I8RS`, `__U8RS` | `long long` `>>` (arithmetic), `unsigned long long` `>>` (logical) | `a` = `AX:BX:CX:DX`, `n` = `SI` | `AX:BX:CX:DX` | `AX`, `BX`, `CX`, `DX` |

Every helper preserves `SI`, `DI`, `BP`, `DS`, `ES` and `SS`, and leaves the direction flag as it found it. Other flags are not preserved. A shift count is the low word of the right operand whatever its type (`int`, `long`, `long long`), and the compiler calls the shift helpers for constant counts too.

The experiments (`wcc` with the consumer flags above, then `wdis -a` of our own objects; also `-od` and `-ox`, which call the helpers the same way):

- **Arguments and results.** `long f(long a, long b) { return a * b; }` compiles to `jmp __I4M`, and likewise for `/` with `__I4D`/`__U4D`: the default convention already has `a` in `DX:AX` and `b` in `CX:BX`, and the result in `DX:AX`. `a % b` is `call __I4D` (or `__U4D`) then `mov ax,bx` / `mov dx,cx`, so the remainder is in `CX:BX`; one helper gives both. For `long long`, `f(a, b)` loads `a` into `AX:BX:CX:DX` (storing `a` through a far pointer shows `AX` is the high word), points `SI` at `b` (`lea si,6[bp]` for a parameter, `mov si,offset _gb` for a global, `mov si,offset L$4` for a constant in `CONST`), calls, and reads the result from `AX:BX:CX:DX`. A shift loads the count into `SI` (`mov si,6[bp]`; `mov si,5` for `a << 5`); for a `long` or `long long` count, only its low word.
- **Registers kept live.** With a far pointer parameter, `p[0] = a; p[1] = a * b; p[2] = b;` keeps `ES:SI` (32-bit) or `ES:DI` (64-bit) live across the call. Loops over arrays (`s += *a++ * *b++`) keep their pointers in `SI` and `DI` across `__I4M` and `__U4D`, and in `DI` across `__I8M` and `__I8RS`. So `SI`, `DI` and `ES` are preserved for the 32-bit helpers, and `DI` and `ES` for the 64-bit ones.
- **Registers not kept.** `a * b + b * c` copies `b` from `CX:BX` to `SI:DI` before `__I4M` rather than reuse `CX:BX`, and a loop reloads the divisor into `CX:BX` before each `__U4D`, so `BX` and `CX` are not relied on after the 32-bit helpers. The 64-bit callers reload `SI` before every call (`a * gb * gb` sets `mov si,offset _gb` twice) and never read it after; `SI` is preserved anyway, since no experiment shows it may change.
- **The 8-byte operand.** After `p[1] = a * b;`, `p[2] = b;` copies `b` from the same stack slot `SI` pointed at, so the helper must not write `[SI]`.

The implementations (`crt/i4m.asm`, `u4d.asm`, `i4d.asm`, `i8m.asm`, `u8d.asm`, `i8d.asm`, `u8div.asm`, `i8ls.asm`, `i8rs.asm`, `u8rs.asm`) give C's result for every defined operand. For undefined ones: a zero divisor raises the divide-error trap (INT 0) in `__I4D` and `__U4D`, and gives a quotient of all ones and the dividend as remainder in the 64-bit divides; `LONG_MIN / -1` and `LLONG_MIN / -1` give the dividend; out-of-range shift counts shift by that many bits one at a time (a count of 64 or more gives 0, or all sign bits).

## Startup

The core startup (`crt/cstart.asm`) needs DOS 2.0. Before `main` it:

- sets `DS` and `SS` to `DGROUP`, keeping the stack size the linker chose (`option stack=n`, default 2048 bytes) (*tested*);
- shrinks the program's memory block to the top of the stack (*tested*);
- zeroes BSS (*tested*) and sets `_psp` to the PSP segment (*tested*);
- runs the init table (below).

When `main` returns, its value is the exit status, through INT 21h function 4Ch (*tested*).

### Init table

Optional members run code before `main` by adding a near function pointer to segment `LIBC16_INIT` (class `DATA`, in `DGROUP`), declaring `LIBC16_INIT_BEGIN`, `LIBC16_INIT` and `LIBC16_INIT_END` in that order. Init routines take no arguments and must preserve `SI`, `DI`, `BP`, `DS` and `SS`; a default-convention C function does. Entries run in link order.

### argv

Linked only when `main` takes parameters. The command tail is split the way Microsoft C splits it (`crt/args.c`) (*tested*). `argv[0]` is `""`.

### DOS 2.0

No DOS 2.0 system is available to run the tests on: Microsoft's MIT release of MS-DOS 2.0 (github.com/microsoft/MS-DOS, `v2.0/bin`) has `MSDOS.SYS` and `COMMAND.COM` but no PC BIOS module to boot them with. DOS 2.0 compatibility is therefore checked against that release's own system-call documentation, `v2.0/bin/SYSCALL.DOC`:

| Startup needs | DOS 2.0 (`SYSCALL.DOC`) |
|---|---|
| Resize the memory block, INT 21h AH = 4Ah, `ES` = block, `BX` = paragraphs | SetBlock, function 4Ah |
| Terminate with a return code, INT 21h AH = 4Ch, `AL` = code | Exit, function 4Ch |
| Command tail at PSP offset 80h | Exec's parameter block: "command line at 80h" |
| `.EXE` programs loaded with a PSP ("program header") | Exec, function 4Bh |

`SYSCALL.DOC` does not describe the registers on entry to an `.EXE` (`DS` = `ES` = PSP, `SS:SP` and `CS:IP` from the header); that is from Ralf Brown's Interrupt List, which gives it for every DOS version. The tests also call INT 21h function 62h (get PSP), which is DOS 3.0; libc16 itself does not. The tests run under DOSBox-X, whose DOS reports version 5.

## Segments

The startup declares `dosseg` ordering. `DGROUP` holds `CONST`, `CONST2`, `_DATA`, the init table, `_BSS`, `STACK` and the empty `LIBC16_STACK_END`, in that order. wlink defines `_edata` (start of BSS) and `_end` (end of BSS); the startup's `___libc16_stack_top` labels the top of the stack, where the heap starts (*tested*).

## Known issues

- wcc writes the absolute path of the source file into each object's module name, so `LIBC16.LIB` depends on the checkout directory and is not yet reproducible across paths. Its translator comment also names the Sybase Open Watcom Public License. Neither reaches a linked program.
