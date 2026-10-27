# libc16 — License and Provenance

libc16's own source is MIT (`LICENSE`). Licensing is a release gate (DESIGN.md §12): every module linked into a program needs an entry here before a release.

## Policy (decision 6)

- Each module has a behavioural spec in `docs/spec/` before it is implemented, citing its sources: the C standard, DOS interrupt documentation, the Microsoft C 5.1 interfaces, Open Watcom *documentation*, and our own call-site experiments (`tests/abi`).
- No one working on libc16 reads Open Watcom runtime source or disassembles its libraries. Compiler support symbols and conventions are found by compiling our own C and disassembling our own objects.
- Startup, arithmetic and 64-bit helpers, the heap and formatting code are implemented by a separate session working from the spec alone.
- Before the first release, an automated audit compares our objects with Open Watcom's runtime library and reports only similarity scores ("Similarity audit" below).
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
| `exit` | `crt/exit.asm` | `docs/spec/startup.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `i4m` | `crt/i4m.asm` | `docs/spec/arith.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `u4d` | `crt/u4d.asm` | `docs/spec/arith.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `i4d` | `crt/i4d.asm` | `docs/spec/arith.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `i8m` | `crt/i8m.asm` | `docs/spec/arith.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `u8div` | `crt/u8div.asm` | `docs/spec/arith.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `u8d` | `crt/u8d.asm` | `docs/spec/arith.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `i8d` | `crt/i8d.asm` | `docs/spec/arith.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `i8ls` | `crt/i8ls.asm` | `docs/spec/arith.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `i8rs` | `crt/i8rs.asm` | `docs/spec/arith.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `u8rs` | `crt/u8rs.asm` | `docs/spec/arith.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `ultoa` | `crt/ultoa.c` | `docs/spec/format.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `ltoa` | `crt/ltoa.c` | `docs/spec/format.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `strtoul` | `crt/strtoul.c` | `docs/spec/format.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `atoi` | `crt/atoi.c` | `docs/spec/format.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `strftime` | `crt/strftime.c` | `docs/spec/time.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `malloc` | `crt/malloc.c`, `crt/heap16.h` | `docs/spec/heap.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `free` | `crt/free.c`, `crt/heap16.h` | `docs/spec/heap.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `realloc` | `crt/realloc.c` | `docs/spec/heap.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `calloc` | `crt/calloc.c` | `docs/spec/heap.md` | Claude (Opus 5.5), separate session; human review pending | Written from the spec |
| `errno`, `memcmp`, `memcpy`, `memmove`, `memset`, `fmemcpy`, `strcat`, `strchr`, `strcmp`, `strcpy`, `strdup`, `strerror`, `stricmp`, `strlen`, `strlwr`, `strncat`, `strncmp`, `strncpy`, `strnicmp`, `strpbrk`, `strrchr` | `crt/*.c` | `docs/spec/string.md` | Claude (Opus 5.5), design session; human review pending | Written from the spec |
| `fdtab`, `doserr`, `open`, `close`, `read`, `write`, `lseek`, `filelen`, `chsize`, `access`, `setmode` | `crt/*.c`, `crt/io16.h` | `docs/spec/io.md` | Claude (Opus 5.5), design session; human review pending | Written from the spec |
| `stdin`, `stdout`, `stderr`, `fopen`, `fclose`, `fflush`, `fgetc`, `fgets`, `fputc`, `fputs`, `fread`, `fwrite`, `ungetc`, `feof`, `ferror`, `getc`, `putc`, `putchar` | `crt/*.c`, `crt/stdio16.h` | `docs/spec/stdio.md` | Claude (Opus 5.5), design session; human review pending | Written from the spec |
| `dosfind`, `findfrst`, `findnext`, `dosopen`, `doscreat`, `dosclose`, `dosread`, `doswrite`, `getfattr`, `setfattr`, `getvect`, `setvect`, `harderr`, `int86`, `int86x`, `intdos`, `intdosx`, `segread`, `bdos`, `kbhit`, `getenv`, `fullpath` | `crt/*.c`, `crt/*.asm`, `crt/dos16.h` | `docs/spec/dos.md` | Claude (Opus 5.5), design session; human review pending | Written from the spec |
| `isalnum`, `isalpha`, `iscntrl`, `isdigit`, `isgraph`, `islower`, `isprint`, `ispunct`, `isspace`, `isupper`, `isxdigit`, `tolower`, `toupper` | `crt/*.c` | `docs/spec/ctype.md` | Claude (Opus 5.5), design session; human review pending | Written from the spec |
| `tzdata`, `tzset`, `timecal`, `localtim` | `crt/*.c`, `crt/time16.h` | `docs/spec/time.md` | Claude (Opus 5.5), design session; human review pending | Written from the spec |
| `dostime`, `stat`, `chmod`, `mkdir`, `utime` | `crt/*.c` | `docs/spec/files.md` | Claude (Opus 5.5), design session; human review pending | Written from the spec |

### Strings, I/O, streams, the DOS interface, character classes, time and files

These members are ordinary library code, not startup, arithmetic helpers, heap or formatting, so decision 6 lets the session that wrote their specs write them too. They were written from `docs/spec/string.md`, `io.md`, `stdio.md`, `dos.md`, `ctype.md`, `time.md` and `files.md`; C99 clauses 7.4, 7.19 and 7.21 and C89 7.12; Microsoft's documentation of `tzset` and `TZ`; the Gregorian calendar and the United States daylight saving rules since 2007; Open Watcom's documentation of `_harderr`; Ralf Brown's Interrupt List for the INT 21h functions they call; Microsoft's documentation of the `io.h`, `fcntl.h`, `dos.h` and `conio.h` interfaces; and experiments in DOSBox-X. No Open Watcom runtime source or header was read and none of its libraries was disassembled. The `strerror` texts are the traditional Unix messages.

### Startup

The startup members were written by a separate session that was given a copy of the repository without `crt/` or its history, and told to work from `docs/spec/startup.md`, `docs/ABI.md`, the tests and public documentation only, and not to read Open Watcom runtime source or disassemble its libraries. Its sources:

- **`cstart`:** the spec; `docs/ABI.md`; Ralf Brown's Interrupt List for the `.EXE` load state and INT 21h functions 4Ah and 4Ch; the 8088's unreliable interrupt hold-off after a move to `SS`; its own experiments, disassembling its own wcc output for segment names, classes and alignments and for `main_`'s registers, and reading wlink maps for `_edata`/`_end`.
- **`argv`:** the spec, `docs/ABI.md` and DESIGN.md decision 8.
- **`exit`:** the spec ("Termination" and "Programs without the startup"), `docs/ABI.md`, `tests/startup/exitst.c` and `ownent`, and INT 21h function 4Ch as Ralf Brown's Interrupt List gives it. A later session, given a copy without `crt/`, its history or GR-DOS's `compat/`, wrote it for M3.
- **`args`:** the spec and `tests/startup/*.expect`; the PSP layout in Ralf Brown's Interrupt List (INT 21h function 26h); Microsoft's documentation of how Microsoft C parses command-line arguments; C99 5.1.2.2.1; Open Watcom documentation of `:>` and `__segment`.

They replace a first version written in the same session that designed the startup, before the spec existed, which decision 6 does not allow for startup code (commits `68b5f10` to `717a884`). That version was not consulted.

### Arithmetic helpers

The members for the compiler's 32- and 64-bit helpers (`i4m`, `u4d`, `i4d`, `i8m`, `u8div`, `u8d`, `i8d`, `i8ls`, `i8rs`, `u8rs`) were written by a separate session working from `docs/spec/arith.md`, `docs/ABI.md`, the existing code and tests, and public documentation, under the same instructions as the startup session: no Open Watcom runtime source, no disassembly of its libraries, no look at its headers, and no GR-DOS runtime code. Its sources:

- **Symbols and register conventions:** its own call-site experiments, compiling one-line C functions and loops with the consumer flags and disassembling the objects with `wdis` (recorded in `docs/ABI.md`, "Compiler support routines").
- **Results:** C99 6.5.5 (truncating division, `(a/b)*b + a%b == a`), 6.5.7 and 6.2.5; the algorithms are textbook ones (schoolbook multiplication by 16-bit words, restoring binary division, two's-complement negation).
- **Expected test results:** `tests/arith/arith.c` compiled for the host, whose C arithmetic gives them.

### Number formatting

`ultoa` and `ltoa` were written by the same separate session, from `docs/spec/format.md` alone: the digit loop is the usual repeated division by the radix, done as two 32-by-16 `div` instructions (Open Watcom documentation of `#pragma aux`) so that neither links a divide helper. The expected test results were written by a short host Perl script from the same values.

For M4 a later separate session, given a copy of the repository holding only `ultoa.c` and `ltoa.c` of the members' sources, wrote `strtoul`, `atoi` and `strftime` from `docs/spec/format.md`, `docs/spec/time.md` and C89 7.10.1.2, 7.10.1.6 and 7.12.3.5, and found and fixed a far-data bug in `ultoa` and `ltoa` (their divide pragma read the value through `DS`, which the compiler had pointed at the caller's buffer). `strtoul` multiplies with a register-only `#pragma aux` so that it links no 32-bit helper. `strftime`'s expected output was written by a host Perl script with Perl's `POSIX::strftime` in the C locale, and by hand from the spec for the boundary cases.

### Heap

`malloc` and `free` were written by the same separate session from `docs/spec/heap.md` and `docs/spec/startup.md`: a first-fit list of blocks with a one-word size header, merging free neighbours during the search, the usual textbook design. Its sources were C99 7.20.3; Ralf Brown's Interrupt List for INT 21h functions 4Ah (resize memory block) and, in the tests, 48h and 49h; and Open Watcom documentation of `#pragma aux`. The same session added `___libc16_stack_top` to `cstart` (an empty segment after `STACK`, as `docs/spec/heap.md` allows), after linking the test programs to check that their maps put it at the top of the stack and that the programs are byte-for-byte unchanged.

For M4 a later separate session, given a copy holding only `malloc.c`, `free.c` and `cstart.asm` of the members' sources, wrote the far heap of the compact and large models, `realloc` and `calloc`, from `docs/spec/heap.md`, `docs/spec/startup.md`, `docs/ABI.md`, C99 7.20.3 and Ralf Brown's Interrupt List (INT 21h function 4Ah). The far heap is a run of arenas of up to one segment each, with the small model's block format, and a mark of the first block that may be free so that a program making many allocations does not search the allocated ones again; the session measured the difference (an 8,000-line test ran past `tests/run-dos`'s 120-second limit without the mark, about 4 seconds with it). The small model's `malloc` and `free` are unchanged.

A later separate session, under the same instructions, added `LIBC16_NULL` to `cstart` (`docs/spec/startup.md`, "Null pointer"): 2 bytes of class `BEGDATA` at the start of `DGROUP`. Its sources were the spec, C99 6.3.2.3 and its own experiments, reading wlink maps of its own objects: a segment of class `DATA` declared first in `cstart` still came after the `DATA` segments of a C object linked before it, while one of class `BEGDATA` came first in `DGROUP` under `dosseg`.

### Memory models (M4)

The compact and large libraries (decision 11) are built from the same sources as the small one. The conventions they follow (`docs/ABI.md`, "Memory models") were found by compiling our own C in each model and disassembling our own objects. Each member kept its author: the design session ported its own assembly members and pragmas (`crt/model.inc`, `int86`, `int86x`, `segread`, `bdos`, `kbhit` and the C members' register pragmas), and separate sessions, each given a copy of the repository holding only its own members' sources and the others as prebuilt objects, ported the startup, the arithmetic helpers, the heap and the number formatting, under the same instructions as before. Each checked that its members' small-model objects did not change.

## Similarity audit

`scripts/similarity-audit` (run by `make audit`, in CI and before each release; written for libc16, MIT) compares the code of every member of each library with the DOS runtime library of the same memory model in the pinned Open Watcom snapshot: `LIBC16.LIB` with `lib286/dos/clibs.lib` and `lib286/maths.lib`, `LIBC16C.LIB` with `clibc.lib` and `mathc.lib`, and `LIBC16L.LIB` with `clibl.lib` and `mathl.lib`, each with the objects in `lib286/dos`. It reads both as OMF records, takes the bytes of code-class segments, leaves out every byte a fixup patches, and cuts the rest into overlapping 8-byte runs. For each libc16 member it prints only the fraction of its runs found in the one Open Watcom member that shares most of them (*best*) and in the whole library (*any*). Nothing is disassembled, and no Open Watcom names, bytes or code are printed or kept. Before scoring, a control check requires every judged member to be found whole in the libc16 library itself. A member with at least 16 runs and a best score above 0.50 fails the audit. Members with fewer runs (a few instructions) are listed but not judged.

Results for 0.03: in `LIBC16.LIB` 83 of the 117 members are long enough to judge, and in `LIBC16C.LIB` and `LIBC16L.LIB` 88. The highest best score in any of them is 0.12 (`stricmp`, small); the next are `i4d` (0.11) and `int86` (0.10). The short members that score higher are those of 0.01 below, and in the compact and large libraries `intdos` (0.53, 15 runs) and `getc` (0.40, 5 runs): a call to `int86` with 21h, and a jump to `fgetc`. `intdosx`, a C call to `int86x` like `intdos` but with one more far pointer, failed the audit in the compact library (0.52, 21 runs) as the compiler's output for that call; it is now written in assembly (`crt/intdosx.asm`), with the small model's instructions unchanged and the far-data arguments copied in a loop, and scores 0.00.

Results for the first release, with the snapshot of decision 1: 42 of the 65 members are long enough to judge. Their highest best score is 0.12 (`stricmp`); most are under 0.05. Four short members score higher: `i8ls` and `i8rs` (0.64, 11 runs), `u8rs` (0.73, 11 runs) and `putc` (1.00, 3 runs). They are a 64-bit shift loop and a jump to `fputc`, which have few possible encodings. The full report is `docs/AUDIT.txt` in each release archive.

## GR-DOS `compat/crt`

Not used. DESIGN.md §12 says it was written after examining Watcom runtime disassembly; GR-DOS's `docs/provenance.md` (its decision 20) says it was written from documented interfaces. The two disagree; this is to be resolved in GR-DOS during M3, when libc16 replaces it.

### GR-DOS's `compat/crt` (M3)

GR-DOS's ATTRIB and MEM linked its own runtime, `compat/crt`, written after the Open Watcom runtime then linked into them had been disassembled (DESIGN.md §12; GR-DOS `docs/provenance.md`, decision 20). libc16 replaces it and does not reuse it. The members those programs needed and libc16 lacked (`int86x`, `intdos`, `intdosx`, `segread`, `bdos`, `kbhit`, `exit`, and `struct SREGS`, `FP_SEG` and `FP_OFF`) were specified and written without reading `compat/crt`, `compat/include` or `compat/msc.h`. GR-DOS's documentation of `compat/crt` (its `docs/m4-utilities.md`) was read only after they were written, to update it. The call sites, in Microsoft's MS-DOS 4.00 sources for ATTRIB and MEM, were read.
