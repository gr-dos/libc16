# libc16 — Design and MVP Plan

**Project:** `gr-dos/libc16`  
**Status:** Accepted design; see §15 for decisions  
**Target:** 16-bit real-mode DOS on Intel 8086/8088-compatible CPUs  
**License goal:** MIT, with independently implemented and auditable source

## 1. Purpose

`libc16` is a small, modular C runtime and support library for the GR-DOS ecosystem. It enables programs compiled with Open Watcom's 16-bit C compiler to run **without linking any Open Watcom runtime implementation**. Its primary consumers are the `gr-dos/*16` utility repositories, followed by the GR-DOS operating-system distribution and its C-based commands.

`libc16` is **not** an attempt to recreate the complete ANSI C library. It grows in response to demonstrated needs of real applications. Nevertheless, each standard C function it exposes must satisfy its documented C-standard contract; each conventional DOS extension should follow an established, documented DOS compiler/runtime interface where practical. Restricted functionality must not masquerade as a standard function.

The overarching invariant is:

> **Unused functionality must impose no code or data cost on a linked executable.**

That includes unused file I/O, argument processing, formatting, allocation, arithmetic helpers, and optional startup facilities. Actual dependencies of selected functions are permitted, but they must be deliberate, minimal and measurable.

## 2. Goals and non-goals

### Goals

- Produce usable, compact `.EXE` programs for original 8086/8088 systems, using a Linux-hosted build workflow.
- Start with **Open Watcom 16-bit C**, **small memory model**, and **Microsoft-compatible `__cdecl`** linkage for library interfaces (decision 2).
- Supply independently authored, permissively licensed implementations of the runtime facilities required by participating tools.
- Enforce explicit linking with `libc16`, with **no fallback** to Open Watcom's runtime.
- Keep archive modules sufficiently granular that adding an unused function never grows other executables.
- Support DOS **2.0** as the baseline for startup and fundamental functions; annotate later DOS requirements per API.
- Make standards compliance, DOS extension compatibility, 8086 safety, binary size, memory use, and provenance testable.
- Integrate the standalone utility ecosystem **before** returning to GR-DOS; both are part of the MVP.
- Maintain a reproducible consumer inventory that expands through automatic repository discovery, including a few explicitly configured exceptions.

### Non-goals for the MVP

- A complete ANSI C library, full floating-point `printf`, a POSIX layer, or broad compiler compatibility.
- DPMI, protected mode, 186/286/386 instruction dependencies, or other memory models.
- `.COM` support initially. Native assembly `.COM` tools remain a separate, valid optimization strategy.
- A universal cross-platform runtime for an eventual Apple IIgs port. Portable routines may be reused or cherry-picked independently.
- Automatically replacing assembly implementations with C implementations.
- Performance benchmarking except where an implementation becomes unacceptably slow.

## 3. Repository and artifact conventions

- Source repository: **`gr-dos/libc16`**.
- Primary static archive: **`LIBC16.LIB`**, compatible with the selected Open Watcom linker/librarian tooling.
- Repository naming: the `gr-dos` organization eliminates the old `dos-` repository prefix (for example, `gr-dos/df16`, not `gr-dos/dos-df16`).
- Build tools initially: Open Watcom (`wcc`, `wlink`, `wlib`), JWasm (decision 4), GNU Make, and Linux-hosted testing/emulators.
- Distribution: versioned releases with binaries, headers, checksums, license/provenance documentation, and reproducible source/build instructions.

Illustrative structure (not a commitment to file-level layout):

```text
libc16/
  include/                  public C and DOS extension headers
  crt/                      minimal EXE startup and optional init modules
  src/
    string/                 one callable routine per archive member where practical
    memory/
    dos/                    DOS-specific wrappers and file handles
    io/
    format/                 lightweight facilities and eventual standard implementations
    compiler/               compiler-generated ABI helpers
  tests/
    abi/
    conformance/
    linking/
    dos/
  consumers/
    manifest.*              discovered + exception consumers; pinned revisions
    discover.*              manifest refresh logic
    runner.*                fixed-input A/B comparisons
  docs/
    ABI.md
    LICENSE-PROVENANCE.md
    CONSUMERS.md
  Makefile
  LICENSE
```

## 4. ABI and compatibility contract

### Initial ABI

- Intel **8086 instruction set only**, including archive members, startup code, and any compiler support helpers.
- DOS real-mode `.EXE`, small code/data model with near pointers where applicable.
- Open Watcom generating code with **Microsoft-compatible `__cdecl`** for published library interfaces, while consumers keep Open Watcom's default register convention (decision 2); document symbol decoration, stack argument placement, return values, register preservation, segment register behavior, struct packing and pointer representation.
- Prefer a public header that explicitly declares ABI details; validate using small compiled/disassembled call-site fixtures rather than relying on assumptions.
- Preserve names expected by Open Watcom for **compiler-generated helper symbols**, even where they are not exposed as public APIs.
- Implement DOS-specific `int86`, `intdos`, `segread`, related structures, and interrupt conventions to match established Microsoft C/DOS conventions when feasible. Document differences explicitly and test segment-register and carry-flag behavior.

### Language/API categories

1. **Standard C functions:** use standard names only with standard-compliant semantics, including edge cases and return values. Avoid ABI claims beyond the documented compiler configuration.
2. **Conventional DOS extensions:** retain familiar DOS names and contracts where an established convention exists; identify compatibility target(s) in documentation.
3. **Project-specific extensions:** use a consistent **`libc16_`** prefix, never a misleading standard-library name. Tiny integer-formatting/output functions may live here as alternatives to a heavyweight `printf` family.
4. **Compiler support routines:** private implementation details with toolchain-mandated symbols; not a public application interface.

Headers must declare **only interfaces actually provided**, distinguish DOS-dependent interfaces from ISO C, and make unsupported combinations fail visibly. Prefer scoped, minimal header definitions over blindly importing an entire Watcom header tree.

## 5. Linking and archive granularity

**Selective linking is the principal architectural acceptance criterion.**

- Package independently callable functions in separate archive members wherever practical. Shared low-level helpers must likewise occupy independent members.
- A function may pull in only the modules necessary to implement its contract. All transitive dependencies must be inspectable via a linker map.
- Optional features (formatting, command-line parsing, environment access, heap, 64-bit arithmetic) cannot be reached from a mandatory initialization object unless every program genuinely requires them.
- Avoid large generic dispatchers where a small direct implementation is sufficient, while preserving correct public semantics.
- **Adding an unused function must not change the byte size or linked-module set of an existing fixture.** Enforce this using a deterministic baseline test with the same compiler/linker inputs.
- Never satisfy unresolved references from a default Open Watcom runtime library. Missing symbols are **build errors**, not permission to fall back.

Example build posture (exact options to be verified and codified by toolchain tests):

```text
wcc ... -x -zl ...           # explicit headers; no default-library records
wlink ... format dos option nodefaultlibs ... LIBC16.LIB
```

The build must produce a map. CI must validate all linked objects/archive members against an allowlist, and reject Open Watcom runtime modules even when an executable otherwise passes its functional tests. An approved compiler or linker executable is not the same as including copyrighted runtime code in the output.

## 6. Startup and process lifecycle

Start with DOS `.EXE` only. A **minimal core startup** must:

- Establish the intended segment register and stack configuration safely on 8086 hardware.
- Identify the PSP and expose it as needed.
- Initialize zero-filled storage/BSS correctly.
- Reserve and, where appropriate, release DOS memory so applications do not needlessly occupy all conventional memory.
- Invoke the application's designated entry point and terminate through DOS with a predictable status contract.

Prefer optional, individually linked modules for:

- `argc`/`argv` construction and command-tail parsing;
- environment access;
- heap setup;
- buffered I/O and other initialization requiring extra state.

Startup features are selected **automatically** from what the compiler references (decision 8): every module defining `main` references `_cstart_`, and one whose `main` takes parameters also references `__argc`. The design must not silently pass unusable `argc`/`argv` to applications that expect them. Declare the startup selection and application-entry contract precisely, including how `main(void)` and `main(int,char **)` are supported.

The core startup targets **DOS 2.0**. Individual routines requiring newer DOS services must specify their own minimum versions without raising that of unaffected applications.

## 7. Functional roadmap driven by consumers

Do **not** implement a feature simply because a conventional libc offers it. First inventory the compiled object references from participating repositories, then implement only the necessary functions and their real dependencies.

| Capability | Examples | Initial stance |
|---|---|---|
| Startup | PSP, BSS, segments, stack, termination | MVP foundation |
| Strings and memory | `strlen`, `strcmp`, `strcpy`, `memcpy`, `memset` | Add on demand; full semantics |
| DOS interfaces | `int86`, `intdos`, segment support | Add on demand; familiar DOS ABI |
| File operations | `open`, `close`, `read`, `write`, `lseek` | Early consumer-driven support |
| CLI/environment | `argc`/`argv`, `getenv` | Optional linkage |
| Integer formatting | small `libc16_*` helpers | Prefer where sufficient |
| Standard stdio | `FILE`, `fopen`, `fread`, `fprintf`, `printf` | Only as consumers require, with honest conformance |
| Arithmetic helpers | Open Watcom 32/64-bit support symbols | Individually linked as required |
| Heap | `malloc`, `free`, `realloc` | Only when required; minimal footprint |

Avoid a partial `sprintf` or `printf` under the standard name. In consumers where formatting needs are trivial, first consider using normal C string operations or small prefixed helpers (as was done when removing `sprintf` from GR-DOS `MEM`). If and when a standard formatting function is offered, define its conformance and ensure that optional conversions do not impose costs on programs that do not call it.

## 8. Consumers and integration order

### Standalone utilities first

Initial discovery scope: repositories matching **`gr-dos/*16`**. Known potential consumers include `hello16`, `truncate16`, `crcsum16`, `df16`, `which16`, `md5sum16`, `tar16`, and `infozip16`; the runner must determine which actually compile and link with libc16. A repository matching the name pattern is **eligible**, not automatically a mandatory active consumer.

**Special-case external consumer:** `tico-edit/tico16`. The central manifest must support a small number of explicitly included external repositories and explicit exclusions, with the same pinned-revision, opt-in build contract used elsewhere. More exceptions can be added, but exceptions should not become the normal discovery mechanism.

`which16` illustrates a tool that may ship a native assembly `.COM` while retaining C as a comparison/reference implementation. The manifest must be able to distinguish **production libc16 consumers**, **test-only libc16 consumers**, **assembly-only/nonparticipating repos**, and **not-yet-migrated** utilities.

A recommended, dependency-driven adoption order is:

1. `hello16` — minimal startup/output and module-isolation benchmark.
2. `truncate16` and `crcsum16` — filesystem I/O and arithmetic paths.
3. `df16` — 64-bit support and integer formatting.
4. Remaining relevant `*16` utilities — extend requirements only as needed.
5. `tico-edit/tico16` — named external consumer, included in discovery from day one; migrate when dependencies can be supported without unnecessary runtime bloat.

This is a **proposed engineering order**, adjustable based on dependency inventory. Utility-specific assembly experiments proceed independently and should be compared on correctness, binary size, memory footprint, and maintenance cost.

### GR-DOS second, still MVP

Once the shared library has been established and exercised through standalone tools, return to `gr-dos/gr-dos`:

- Replace its existing `compat/crt` runtime dependency with independently developed `libc16` implementations, initially for ATTRIB and MEM.
- Preserve reference builds and emulator/physical-hardware compatibility expectations.
- Compare linker maps, size, disk cluster usage, runtime memory, and functional output with the already optimized GR-DOS binaries.
- Audit other C-based DOS utilities incrementally; do not make a complete port of every command a condition for the initial libc16 release unless an MVP consumer genuinely needs it.

**Both phases are in the MVP; “standalone utilities first” is ordering, not a decision to defer GR-DOS past the MVP.**

## 9. Dependency management and reproducible builds

Consumers use both:

- **Pinned-release acquisition (default):** fetch a versioned `libc16` release artifact and verify its checksum; never use an unpinned moving branch for releases or CI.
- **Local override:** an explicit variable such as `LIBC16_ROOT` points at an installed or in-development build, supporting fast local iteration and A/B CI comparisons.

The exact variable names and Make targets belong to the initial integration-contract design; they must be common and documented. Keep the build entirely possible under Linux and preserve consumer independence. Avoid mandatory Git submodules.

Use fixed toolchain versions and the **same consumer commit** when comparing two libc16 versions. Record compiler/linker/assembler versions, build flags, library revisions, and consumer commit hashes alongside measurements.

## 10. Consumer manifest and automatic discovery

A **central manifest** inside `gr-dos/libc16` is the authoritative reproducible inventory. An automated discovery tool periodically enumerates `gr-dos` repositories, proposes additions for newly eligible `*16` repositories, and considers explicit external includes such as `tico-edit/tico16`.

The manifest must support:

- Organization pattern (`gr-dos/*16`) and explicitly included external repositories.
- Explicit exclusions and participation states (`active`, `pending`, `test-only`, `not-applicable`).
- A pinned commit/revision, expected output artifact(s), and a standardized local build/test invocation.
- A source revision update workflow separate from library-change validation.
- Special build instructions only where unavoidable; prefer a standard Make target or a simple metadata contract published by the consumer.

Discovery should **propose** manifest updates (for review/automation PR), not cause a running regression test to follow changing branch heads. Not every new `*16` repository is assumed to use C or libc16.

### Two complementary regression modes

**A. Library-change validation:** for each *active* consumer, build the **same pinned source revision** against the approved baseline libc16 and the proposed libc16; use the same toolchain. Compare tests, executable bytes, conventional memory, and linked archive members. This isolates changes due to the library.

**B. Ecosystem inventory refresh:** discover newly eligible repositories, update pinned consumer revisions and baselines through a deliberate change, and make the suite grow as tools are added. Report pending/nonparticipating repos without treating their current Watcom-runtime linkage as a migrated-consumer regression.

## 11. Measurements, CI and acceptance policies

For each participating executable, record:

- Absolute binary size and difference against the baseline (bytes and percent).
- Allocation in common FAT12 cluster sizes, especially 1 KiB clusters on 360/720 KiB floppies, as well as 512-byte clusters where relevant.
- Resident/load-time conventional-memory requirements where measurable; distinguish `.COM` allocation behavior from useful footprint.
- Library/archive modules linked, mandatory and transitive dependencies, and any unexplained additions.
- Compatibility matrix: 8086 instruction constraints, minimum DOS version, and emulator/hardware coverage.
- Functional/unit/differential results, including edge cases, errors and return statuses.
- License/provenance status for incorporated runtime implementation code.

CI must **fail** for migrated/active consumers if:

- Any Open Watcom runtime code or unapproved library is linked.
- An unresolved reference is quietly satisfied by an implicit/default library.
- Required functional or compatibility tests fail.
- A fixture that does **not** use a newly added feature gains linked modules or changes binary size solely because the feature was added.

Size increases are **reported and subject to documented review/budgets**, not automatically forbidden: correct standard behavior may cost bytes. Disallow hidden runtime dependencies rather than enforcing an arbitrary perpetual shrinking target. New active consumers enter the size report automatically when their manifest entry is accepted; existing consumer baselines remain comparable against pinned revisions.

Static checks are necessary but not sufficient: exercise the produced binaries under DOS emulation and, when available, on real 8088/8086 hardware.

## 12. Licensing and clean implementation

MIT is the intended license of independently authored `libc16` source. **Licensing is a release gate, not a paperwork task deferred until after functionality works.**

- Implement from documented C/DOS interfaces, public ABI descriptions and independently produced compiler-call-site experiments.
- Do not copy, mechanically translate, or relabel Open Watcom runtime source or disassembly.
- Document implementation provenance for each module and any third-party material and notices.
- The existing GR-DOS runtime was written after its implementer examined Watcom runtime disassembly. That history must be disclosed and evaluated; **do not simply transplant it and declare its MIT provenance settled**. Prefer fresh independent implementation or a well-supported review/clean-room approach where required.
- Validate actual linked module contents and maps; removing a notice string does not establish absence of copyrighted runtime code.
- Keep compiler/tool use and incorporation of compiler-provided *runtime code* conceptually and legally distinct.
- If any included code's rights or provenance are uncertain, stop that integration and resolve the concern before publishing an MIT-only binary claim.

## 13. Milestones and MVP exit criteria

### M1 — Foundation

- [ ] Create repository, MIT license/provenance process, toolchain-pinned Linux build and static `LIBC16.LIB` output.
- [ ] Define and test `__cdecl` small-model ABI and DOS 2.0 minimum startup behavior.
- [ ] Establish link-map allowlisting; tests prove no Open Watcom runtime objects can enter outputs.
- [ ] Prove selective module extraction and zero size impact from an unused newly added function.

### M2 — Standalone utility integrations

- [ ] Integrate `hello16`, then additional eligible utility repos according to dependency inventory.
- [ ] Provide pinned release download and local-library override with common build/test interface.
- [ ] Provide standard-compliant required functions and private compiler helper routines as needed.
- [ ] Install central manifest + automatic `gr-dos/*16` discovery + explicit `tico-edit/tico16` inclusion.
- [ ] Implement library A/B regression reports with fixed consumer revisions and licensing/size/memory checks.
- [ ] Migrate relevant active consumers off Open Watcom runtime; pending or assembly-only entries clearly identified.

### M3 — GR-DOS integration (still MVP)

- [ ] Replace the existing GR-DOS runtime in targeted C commands with `libc16` using independently validated code.
- [ ] Retain 8086, DOS and reference-build compatibility, plus test coverage for `ATTRIB.EXE` and `MEM.EXE`.
- [ ] Demonstrate no fallback to Watcom libraries and measure effects on `.EXE` sizes, runtime memory and boot-floppy cluster consumption.
- [ ] Publish release documentation, consumer manifest, build artifacts and reproducibility/provenance notes.

### MVP is complete when

1. At least representative standalone `*16` C utilities build and test with `libc16` **without Open Watcom runtime code**.
2. The repository includes a tested minimal `.EXE` startup, standards-compliant implemented C APIs, and separately extractable object members.
3. The pinned/override consumer workflow and growing discovery/manifest regression runner are operational, including the exceptional external repository configuration.
4. A GR-DOS integration replaces the existing runtime for the agreed initial commands and passes its relevant compatibility, licensing and size checks.
5. Every shipped module has documented permission to redistribute under the project's declared licensing terms.

## 14. Deferred design details to validate experimentally

These need implementation evidence, not further top-level policy debate:

- Precise compiler/linker flags and calling convention/segment rules for each public DOS extension.
- The smallest conforming I/O API required by the first migrated utility, especially how to avoid unnecessarily dragging in general `stdio`.
- Which arithmetic helper symbols are emitted by each participating consumer and how to isolate them.
- Consumer metadata schema and manifest file format; the behavior of discovery/pinning matters more than YAML versus another format.
- Version/ABI compatibility policy after the first stable release and how downstream consumers pin updates.

These questions may change implementation details, but not the agreed platform, modularity, licensing, consumer ordering, or testability principles.

## 15. Decisions

1. **Open Watcom version.** Pin open-watcom-v2 release `2026-10-01-Build`, asset `ow-snapshot.tar.xz` (`sha256:e6aa1b1e40ac8bbf97658d2c70fff8a4242d6ca4a1c60806f2baa5317083d4fe`), the same pin as GR-DOS (its decision 10). Change it only together with GR-DOS.
2. **Calling convention.** Consumers compile with Open Watcom's default register convention. Every public libc16 function is declared `__cdecl` in its header: Microsoft-style `_name` symbols, stack arguments, caller pops. Callback parameters (`qsort`, `bsearch`, `atexit`) take `__cdecl` function-pointer types, so a mismatched callback fails to compile. Startup calls the application as `main_` under the register convention. Compiler support routines keep the conventions Open Watcom mandates for them. GR-DOS drops `-ecc` when it moves to libc16 (M3).
3. **Standard scope.** Implement only what consumers need, and give each standard function it provides its full standard contract. 64-bit integer support is required (`df16`). For simple formatting, use `libc16_` helpers instead of `printf`/`snprintf`.
4. **Assembler.** JWasm v2.20, built by the same fetch script and patches as GR-DOS (its decision 19), run without `-Zj`. JWasm is under the Sybase Open Watcom Public License, acceptable for a build tool. Every source declares `.8086`. Use assembly only where C cannot do the job: startup, interrupt interfaces and compiler support routines.
5. **8086 verification.** Both a static opcode scanner over every archive member and linked executable, and execution under an emulator that models an 8086/8088. The emulator, and which runs happen in CI rather than as a documented manual step, are settled in M1.
6. **Provenance.** Each module gets a short behavioral spec before it is implemented, citing its sources: the C standard, DOS interrupt documentation, the Microsoft C 5.1 interfaces, Open Watcom *documentation* and our own call-site experiments. Compiler support symbols and conventions are found by compiling our C and disassembling our own objects. No one working on libc16 reads Open Watcom runtime source or disassembles its libraries. `docs/LICENSE-PROVENANCE.md` records each module's authors (including AI assistance, also recorded in commit trailers), sources, spec and tests. Startup, arithmetic and 64-bit helpers, the heap and any formatting code are implemented by a separate session from the spec alone. Before the first release, an automated similarity audit compares our objects with Open Watcom's runtime library and reports only scores. The GR-DOS `compat/crt` runtime is replaced, not reused, and the conflict between §12 and GR-DOS's `docs/provenance.md` (decision 20 there) about how it was written is resolved in M3.
7. **CI and discovery.** GitHub Actions. Discovery uses the GitHub API through `gh` and proposes manifest changes as pull requests.
8. **Startup selection.** Automatic. The core startup defines `_cstart_`; a separate archive member defines `__argc`/`__argv`, so it is linked only when `main` takes parameters. Optional members register their initialization (and later termination, such as flushing buffered I/O) by contributing entries to an init table in a dedicated segment, which the startup walks without referencing any of them.
9. **Text mode.** `open` defaults to `O_TEXT` with CR/LF translation, following Microsoft C. Ctrl-Z handling is specified with the first consumer that reads text.
10. **Delivery.** M1 to M3 are delivered as small, focused commits.
