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
| `cstart` | `crt/cstart.asm` | `docs/spec/startup.md` | Claude (Opus 5.5); human review pending | **Not yet compliant**, see below |
| `argv` | `crt/argv.asm` | `docs/spec/startup.md` | Claude (Opus 5.5); human review pending | **Not yet compliant** |
| `args` | `crt/args.c` | `docs/spec/startup.md` | Claude (Opus 5.5); human review pending | **Not yet compliant** |

### Startup: deviation from decision 6

The startup members were written in the same session that settled their design, before their spec existed, which the policy does not allow for startup code. They were written from DOS documentation, the Microsoft C argument-parsing rules and our own experiments; no Open Watcom runtime source or disassembly was consulted. The init table's segments were first named `XIB`/`XI`/`XIE`, names other runtimes use, and were renamed to `LIBC16_INIT_*` (commit `717a884`).

**To resolve:** a separate session reimplements the three members from `docs/spec/startup.md` alone, without seeing `crt/`, and its implementation replaces these once it passes `make test`.

## GR-DOS `compat/crt`

Not used. DESIGN.md §12 says it was written after examining Watcom runtime disassembly; GR-DOS's `docs/provenance.md` (its decision 20) says it was written from documented interfaces. The two disagree; this is to be resolved in GR-DOS during M3, when libc16 replaces it.
