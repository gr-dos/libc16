# libc16 — License and Provenance

libc16's own source is MIT (`LICENSE`). Licensing is a release gate (DESIGN.md §12): every module linked into a program needs an entry here before a release.

## Policy (decision 6)

- Each module has a behavioural spec in `docs/spec/` before it is implemented, citing its sources: the C standard, DOS interrupt documentation, the Microsoft C 5.1 interfaces, Open Watcom *documentation*, and our own call-site experiments (`tests/abi`).
- No one working on libc16 reads Open Watcom runtime source or disassembles its libraries. Compiler support symbols and conventions are found by compiling our own C and disassembling our own objects.
- Startup, arithmetic and 64-bit helpers, the heap and formatting code are implemented by a separate session working from the spec alone.
- Before the first release, an automated audit compares our objects with Open Watcom's runtime library and reports only similarity scores.
- AI assistance is recorded here and in the `Co-Authored-By` trailers of the commits. Material generated purely by an AI may not be protected by copyright; the MIT grant covers the human contributions, and nothing here restricts redistribution of the rest.

## Tools (not linked into programs)

| Tool | License | Notes |
|---|---|---|
| Open Watcom C `2026-10-01-Build` | Sybase Open Watcom Public License | Compiler, `wlink`, `wlib`, `wdis`. Its translator comment, naming the license, is in each C object in `LIBC16.LIB` but not in linked programs. |
| JWasm v2.20 with `patches/jwasm` | Sybase Open Watcom Public License | Patches from GR-DOS, MIT. |
| DOSBox-X | GPL-2.0 | Runs tests only. |

## Modules

| Member | Source | Spec | Authors | Status |
|---|---|---|---|---|
| `cstart` | `crt/cstart.asm` | `docs/spec/startup.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `argv` | `crt/argv.asm` | `docs/spec/startup.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `args` | `crt/args.c` | `docs/spec/startup.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |

### Startup

The startup members were written by a separate session that was given a copy of the repository without `crt/` or its history, and told to work from `docs/spec/startup.md`, `docs/ABI.md`, the tests and public documentation only, and not to read Open Watcom runtime source or disassemble its libraries. Its sources:

- **`cstart`:** the spec; `docs/ABI.md`; Ralf Brown's Interrupt List for the `.EXE` load state and INT 21h functions 4Ah and 4Ch; the 8088's unreliable interrupt hold-off after a move to `SS`; its own experiments, disassembling its own wcc output for segment names, classes and alignments and for `main_`'s registers, and reading wlink maps for `_edata`/`_end`.
- **`argv`:** the spec, `docs/ABI.md` and DESIGN.md decision 8.
- **`args`:** the spec and `tests/startup/*.expect`; the PSP layout in Ralf Brown's Interrupt List (INT 21h function 26h); Microsoft's documentation of how Microsoft C parses command-line arguments; C99 5.1.2.2.1; Open Watcom documentation of `:>` and `__segment`.

They replace a first version written in the same session that designed the startup, before the spec existed, which decision 6 does not allow for startup code (commits `68b5f10` to `717a884`). That version was not consulted.

## GR-DOS `compat/crt`

Not used. DESIGN.md §12 says it was written after examining Watcom runtime disassembly; GR-DOS's `docs/provenance.md` (its decision 20) says it was written from documented interfaces. The two disagree; this is to be resolved in GR-DOS during M3, when libc16 replaces it.
