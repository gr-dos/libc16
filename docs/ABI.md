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

Calls the compiler emits for operations it does not inline use Open Watcom's own symbols and register conventions. They are private to libc16 and implemented as consumers need them. Observed so far (*tested*):

| Operation | Symbol |
|---|---|
| 32-bit signed multiply | `__I4M` |
| 32-bit unsigned divide | `__U4D` |
| 64-bit multiply | `__I8M` |
| 64-bit signed divide | `__I8DQ` |

Their exact register contracts are worked out, by experiment, when each is implemented (decision 6).

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

## Segments

The startup declares `dosseg` ordering. `DGROUP` holds `CONST`, `CONST2`, `_DATA`, the init table, `_BSS` and `STACK`, in that order. wlink defines `_edata` (start of BSS) and `_end` (end of BSS).

## Known issues

- wcc writes the absolute path of the source file into each object's module name, so `LIBC16.LIB` depends on the checkout directory and is not yet reproducible across paths. Its translator comment also names the Sybase Open Watcom Public License. Neither reaches a linked program.
