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
| `getch`, `putch` | `crt/getch.asm`, `crt/putch.asm` | `docs/spec/dos.md` | Claude (Opus 5.5), the GR-DOS FDISK port session; human review pending | Written from the spec; see "`getch` and `putch`" |
| `rename`, `unlink`, `remove`, `fseek`, `ftell` | `crt/*.c` | `docs/spec/files.md`, `docs/spec/stdio.md` | Specs and tests: Claude (Opus 5.5), design session. Code: Claude (Opus 5.5), separate session. Human review pending | Written from the spec; see "`rename`, `unlink`, `remove`, `fseek` and `ftell`" |
| `strtol`, `strcspn`, `strstr`, `qsort`, `abort`, `clearerr`, `isascii`, `fstat`, `getcwd`, `chdir` | `crt/*.c` | `docs/spec/format.md`, `string.md`, `stdlib.md`, `stdio.md`, `ctype.md`, `files.md` | Specs and tests: Claude (Opus 5.5), design session. Code: Claude (Opus 5.5), separate session. Human review pending | Written from the spec; see "The Tcl16 functions" |
| `atexit`, `rand`, `srand`, `time`, `gmtime`, `fromloc`, `isatty`, `dup`, `dup2`, `fdopen`, `streams`, `rmdir`, `strupr`; changes to `cstart`, `exit`, `fopen` and `timecal` | `crt/*.c`, `crt/*.asm` | `docs/spec/startup.md`, `stdlib.md`, `time.md`, `io.md`, `stdio.md`, `files.md`, `string.md` | Specs and tests: Claude (Opus 5.5), design session. Code: Claude (Opus 5.5), separate session. Human review pending | Written from the spec; see "The perl16 functions" |
| `setjmp`, `longjmp` | `crt/setjmp.asm`, `crt/longjmp.asm` | `docs/spec/setjmp.md` | Specs and tests: Claude (Opus 5.5), design session. Code: Claude (Opus 5.5), separate session. Human review pending | Written from the spec; see "`setjmp` and `longjmp`" |
| `system`, `spawnl`, `spawnv`, `spawnve`, `spawnvp`, `spawnvpe`, `getpid`, `spawnx`, `spawnp`, `dosexec` | `crt/*.c`, `crt/dosexec.asm`, `crt/proc16.h` | `docs/spec/process.md` | Specs and tests: Claude (Opus 5.5), design session. Code: Claude (Opus 5.5), separate session. Human review pending | Written from the spec; see "Running programs" |
| `signal`, `raise`, `sigtab`, `sigint`; `abort` rewritten | `crt/signal.asm`, `crt/raise.asm`, `crt/sigtab.asm`, `crt/sigint.asm`, `crt/abort.asm` | `docs/spec/signal.md`, `docs/spec/stdlib.md` | Specs and tests: Claude (Opus 5.5), design session. Code: Claude (Opus 5.5), separate session. Human review pending | Written from the spec; see "Signals" |
| Changes to `cstart` and `sigtab` (the code reserve) | `crt/cstart.asm`, `crt/sigtab.asm` | `docs/spec/startup.md` | Specs and tests: Claude (Opus 5.5), design session. Code: Claude (Opus 5.5), separate session. Human review pending | Written from the spec; see "The code reserve" |
| `isalnum`, `isalpha`, `iscntrl`, `isdigit`, `isgraph`, `islower`, `isprint`, `ispunct`, `isspace`, `isupper`, `isxdigit`, `tolower`, `toupper` | `crt/*.c` | `docs/spec/ctype.md` | Claude (Opus 5.5), design session; human review pending | Written from the spec |
| `tzdata`, `tzset`, `timecal`, `localtim` | `crt/*.c`, `crt/time16.h` | `docs/spec/time.md` | Claude (Opus 5.5), design session; human review pending | Written from the spec |
| `dostime`, `stat`, `chmod`, `mkdir`, `utime` | `crt/*.c` | `docs/spec/files.md` | Claude (Opus 5.5), design session; human review pending | Written from the spec |
| `setvbuf`, `setbuf`, `bufio`, `bufexit`, `bufhook`; buffering in `fputc`, `fputs`, `fwrite`, `fflush`, `fclose`, `fseek`, `ftell`, `fgetc`, `fread`, `system` and `spawnx` | `crt/*.c`, `crt/bufexit.asm`, `crt/stdio16.h` | `docs/spec/stdio.md` | Claude (Opus 5.5), one session for spec, tests and code; human review pending | Written from the spec; see "Buffered output" |

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

For #9, the design session found from libc16's own link maps that the word-aligned `LIBC16_NULL` started up to 14 bytes into `DGROUP`'s frame, so that `DS:0000` was the end of the code. Its experiment linked an empty paragraph-aligned contribution to `LIBC16_NULL`. It then specified a paragraph-aligned `LIBC16_NULL` (`docs/spec/startup.md`, "Null pointer") and wrote `tests/startup/nullodd` and the stricter `check-null`. A separate session, given a copy of the repository without history and under the same instructions, made the change in `cstart` (its alignment only; `wdis` shows the object otherwise unchanged).

### Memory models (M4)

The compact and large libraries (decision 11) are built from the same sources as the small one. The conventions they follow (`docs/ABI.md`, "Memory models") were found by compiling our own C in each model and disassembling our own objects. Each member kept its author: the design session ported its own assembly members and pragmas (`crt/model.inc`, `int86`, `int86x`, `segread`, `bdos`, `kbhit` and the C members' register pragmas), and separate sessions, each given a copy of the repository holding only its own members' sources and the others as prebuilt objects, ported the startup, the arithmetic helpers, the heap and the number formatting, under the same instructions as before. Each checked that its members' small-model objects did not change.

### `fopen`'s and `fdopen`'s update modes

The update modes (0.17) are for `gr-dos/tcl16`, whose `open` passes `r+`, `w+` and `a+` to `fopen`. One session wrote the "Update streams" section of `docs/spec/stdio.md` and `tests/stdio/update.c` first, then changed `crt/fopen.c` and `crt/fdopen.c` from that spec: each now reads `+` and `b` (or `t`) in either order and opens for reading and writing. Rebased onto buffered output (0.16), `crt/setvbuf.c` allocates no buffer for an update stream, which `__libc16_bufset` already treats as a read stream. Stream code is not among the modules decision 6 gives to a separate implementation session. Its sources were C89 7.9.5.3 (update streams) and libc16's own stdio and I/O spec, members and tests. It read no Open Watcom runtime source or other C library's source, and disassembled no library but libc16's own objects, for their sizes.

### `libc16_systeme`

`libc16_systeme` (0.15, #31) is for `gr-dos/perl16`, which runs shell commands containing double quotes with `system` and so loses its changes to `%ENV`. It was written clean-room, in two sessions, as the spawn functions were:

- **Design session.** It wrote the `libc16_systeme` section of `docs/spec/process.md`, the declaration in `<process.h>`, `tests/process/systeme.c` and its cases, and the change to `tests/process/child.c` that prints the raw command tail, committed before any code. Its sources: libc16's own `system` and spawn spec and members; Ralf Brown's Interrupt List for INT 21h 4B00h, 48h and 49h; gr-dos/libc16#31; and its own DOSBox-X experiments, which ran the tests against a throwaway stand-in calling `__libc16_dosexec` to check their expected output, and which was not committed or shown to the implementation session. It read no Perl, Open Watcom runtime or other C library source.
- **Implementation session.** It worked in a copy of the repository without git history, told to read nothing outside it. It reported reading only the process spec, the process members and their headers, `include/process.h`, the tests, the Makefile and two docs for references, and running the tools on libc16's own objects. It moved the environment block out of `spawnx` into a new private member, `envblock`, made `system`'s member hold a core shared with `libc16_systeme` (`crt/systeme.c`, a one-line wrapper), and added `envblock` to the members files of the programs that call a spawn function. It changed no spec or test otherwise.

No Open Watcom runtime source or header was read and none of its libraries was disassembled.

### Buffered output

Buffered stream output (0.16, #34) is for `gr-dos/perl16`, whose clean-runtime build was about half again as slow as its Open Watcom build on a print-heavy workload, largely because each output call was a DOS write. It is ordinary library code, not startup, arithmetic helpers, heap or formatting, so decision 6 lets one session write the spec, the tests and the code, and one did: the spec ("Buffering" and the changes to "Functions" and "File positions" in `docs/spec/stdio.md`, and the related lines of `process.md`, `startup.md` and `stdlib.md`), the header declarations and the tests were committed before the code. Its sources: C89 7.9.3, 7.9.5.1, 7.9.5.2, 7.9.5.5, 7.9.5.6 and 7.10.4.3 and C99 7.19.3; Ralf Brown's Interrupt List for INT 21h functions 40h and 42h; libc16's own specs and members; gr-dos/libc16#34; and, of `gr-dos/libkitchen16`, `docs/spec/fmtio.md` and `src/fprintf.c`, to confirm that its formatted output writes through libc16's `fwrite`. It read no Perl, Open Watcom runtime or other C library source and disassembled none of their libraries. The termination routine (`bufexit`) is an entry in the existing termination table; `cstart` and `exit` are unchanged.

The similarity audit scores the new members at most 0.07 (`bufio`, compact and large); `fflush`, now 25 to 29 runs, scores 0.16 (compact) and 0.14 (large), and `setbuf` 0.11 (compact): short members of a test and a call through a pointer.

### The heap's scaling

0.14 (#30) rewrites `malloc`, `free` and `realloc` so that their time does not grow with the number of blocks, for `gr-dos/perl16`'s Perl 3, whose hash and recursion tests took minutes with 0.12. It was written clean-room, in two sessions, as the earlier heap was:

- **Design session.** It wrote the "Performance" section and the space and `realloc` requirements of `docs/spec/heap.md`, `tests/heapperf/scale.c`, `tests/heap/random.c` and the in-place checks in `tests/heap/grow.c`, committed before any code, and measured 0.12 with them under DOSBox-X. Its sources: C99 7.20.3, libc16's own heap spec and members, and gr-dos/libc16#30. It read no Perl, Open Watcom runtime or other C library source. After the implementation session reported it, it added `i4m` to `random`'s members (its generator's 32-bit multiply).
- **Implementation session.** It worked in a copy of the repository without git history, told to read nothing outside it, including other C libraries and Open Watcom's runtime source, headers and libraries. It reported reading only the specs, `docs/ABI.md`, `docs/DESIGN.md`, the policy and heap rows of this file, the old heap members, the tests, the Makefile and other `crt` members for conventions, and running the tools on libc16's own objects. It rewrote `crt/heap16.h`, `malloc.c`, `free.c` and `realloc.c` from the textbook techniques it recalled: segregated free lists and Knuth's boundary tags (TAOCP vol. 1, 2.5). It changed no spec or test. `calloc` is unchanged.

No Open Watcom runtime source or header was read and none of its libraries was disassembled.

### `getch` and `putch`

`getch` and `putch` (0.04) are for GR-DOS's FDISK, a Microsoft C 5.1 program from the MS-DOS 4.00 source. They were specified in `docs/spec/dos.md` from Microsoft's documentation of the `conio.h` interface and Ralf Brown's Interrupt List for INT 21h functions 02h, 06h and 08h, and written from that spec. No Open Watcom runtime source or header was read and none of its libraries was disassembled.

Unlike the other members, these were written by a session that had looked at Microsoft's code for the same functions. Porting FDISK to GR-DOS, that session disassembled Microsoft C 5.1's `getch` and `putch` as linked into Microsoft's FDISK.EXE (the MS-DOS 4.00 reference build) to learn which DOS function each calls: 08h and 06h. libc16's members are each a single DOS call, and its `putch` differs from what was seen: it writes 0FFh with function 02h, and there is no `ungetch` buffer. The session used no other part of that code.

### `rename`, `unlink`, `remove`, `fseek` and `ftell`

These (0.05) are for `gr-dos/patch16`, a port of GNU `patch` 2.2 (GPL). An earlier version of them, in gr-dos/libc16#11, was written by the session that was porting GNU `patch` and had read parts of its code. These replace it and were written clean-room, in two sessions:

- **Design session.** It wrote `docs/spec/files.md` ("Names with wildcards" to "Headers"), `docs/spec/stdio.md` ("File positions"), the header declarations, and `tests/files/rename.c` and `tests/stdio/seek.c`, committed before any code. Its sources: C89 7.9.4.1, 7.9.4.2, 7.9.7.11, 7.9.9.2 and 7.9.9.4; Microsoft's documentation of `rename`, `remove`, `unlink`, `fseek` and `ftell`; Ralf Brown's Interrupt List for INT 21h functions 19h, 41h and 56h; and libc16's own `docs/spec/io.md` and stream code. It read the description of #11 and its provenance paragraph, but none of #11's code, specs or tests, and none of GNU `patch`'s or `tico-edit/tico16`'s source.
- **Implementation session.** It was given a copy of the repository without its git history, so without #11, and told to read nothing outside that copy: not libc16's checkout, not `patch16`, `tico16` or any other repository, no other C library's source, and no Open Watcom runtime source, headers or libraries. It reported reading nothing outside the copy. Its sources: the specs and tests; libc16's existing `crt/` members, which it did not change; C89 and Ralf Brown's Interrupt List from memory; and its own DOSBox-X experiments. Those experiments found that DOSBox-X deletes a read-only file with function 41h, reports an empty directory as not found, and returns error 5 from 56h when the new name's directory is missing. So `unlink` and `remove` check the attributes before deleting, and `rename` checks the new name's directory after an error 5. Text-mode `ftell` walks back over the unread bytes in the buffer; for each `\n` it reads the file byte before it to see whether it is a `\r`, then restores the handle.

After review, the design session specified what a DOS error in `fseek` or `ftell` leaves (`docs/spec/stdio.md`, "Seek errors") and added tests that make each of their DOS seeks fail (`tests/io/seekfail.asm`). The implementation session, in the same copy and under the same instructions, then changed `fseek` and `ftell` to move the handle back and keep the stream as it was. The shared code is in `__libc16_seekback`, in `ftell`'s member.

No Open Watcom runtime source or header was read and none of its libraries was disassembled.

### The Tcl16 functions

`strtol`, `strcspn`, `strstr`, `qsort`, `abort`, `clearerr`, `isascii`, `fstat`, `getcwd` and `chdir` (0.06) are for `gr-dos/tcl16` (#12), which ports Tcl 6.7 and Tiny Tcl. They were written clean-room, in two sessions, as `rename` and the others were:

- **Design session.** It wrote the specs (`docs/spec/format.md`, `string.md`, the new `stdlib.md`, `stdio.md`, `ctype.md` and `files.md`), the header declarations and the tests, committed before any code. Its sources: C89 7.3, 7.9.10.1, 7.10.1.5, 7.10.4.1, 7.10.5.2, 7.11.5.3 and 7.11.5.7; Microsoft's documentation of `abort`, `isascii`, `fstat`, `getcwd` and `chdir`; Ralf Brown's Interrupt List; and libc16's own specs. It checked `tests/format/strtol`'s expected values with the host's `strtoll`, limited to 32 bits. Of `gr-dos/tcl16` it read only the M1 and M2 reports, for the list of missing functions; it read no Tcl, Tiny Tcl, Open Watcom runtime or other C library source.
- **Implementation session.** It worked in a copy of the repository without git history, told to read nothing outside it, including any other repository, Tcl source, other C libraries and Open Watcom's runtime source, headers and libraries. It reported reading nothing outside the copy. Its sources: the specs and tests; libc16's existing members, which it did not change; C89 and Ralf Brown's Interrupt List from memory; textbook heapsort; and its own DOSBox-X experiments.

No Open Watcom runtime source or header was read and none of its libraries was disassembled.

### The perl16 functions

`atexit` (with a termination table walked by `cstart` and `exit`), `rand`, `srand`, `time`, `gmtime`, `isatty`, `dup`, `dup2`, `fdopen`, `rmdir` and `strupr` (0.08) are for `gr-dos/perl16` (#22), which ports Perl 3 and listed them in its M3 runtime audit. They were written clean-room, in two sessions, as the Tcl16 functions were:

- **Design session.** It wrote the specs (`docs/spec/startup.md`, `stdlib.md`, `time.md`, `io.md`, `stdio.md`, `files.md`, `string.md` and `docs/ABI.md`), the header declarations and the tests, committed before any code. Its sources: C89 7.10.2, 7.10.4.1 to 7.10.4.3, 7.12.2.4 and 7.12.3.3; POSIX.1-2008 for `fdopen` and `dup2`; Microsoft's documentation of `isatty`, `dup`, `dup2`, `fdopen`, `rmdir` and `strupr`; Ralf Brown's Interrupt List; the generator constants of *Numerical Recipes in C* and Knuth; and its own DOSBox-X experiments with INT 21h 2Bh, 2Dh, 3Ah, 4400h, 45h and 46h. It computed the tests' expected `gmtime`, `time` and `rand` values with the host's Perl. Of `gr-dos/perl16` it read only `docs/M3-RUNTIME-AUDIT.md`, for the list of functions and their requirements; it read no Perl, Open Watcom runtime or other C library source. After the implementation session reported them, it fixed three of its tests (missing `<stdio.h>`, DOSBox-X reading a clock set at .00 back early, and the order of the init and termination segments) and the spec's segment order.
- **Implementation session.** It worked in a copy of the repository without git history, told to read nothing outside it, including any other repository, other C libraries and Open Watcom's runtime source, headers and libraries. It reported reading nothing outside the copy, other than running the tools. Its sources: the specs and tests; libc16's existing members; C89, POSIX and Ralf Brown's Interrupt List from memory; and its own experiments, disassembling only its own objects. It changed `cstart` (one walker for the init and termination tables), `exit`, `fopen` (its stream table moved to `streams`, shared with `fdopen`) and `timecal` (`__libc16_from_local` moved, unchanged, to `fromloc`, so that `gmtime` does not link `tzset`).

No Open Watcom runtime source or header was read and none of its libraries was disassembled.

### `setjmp` and `longjmp`

`setjmp` and `longjmp` (0.09) are for `gr-dos/perl16` (#19), whose `die`, `eval` and loop control jump across many frames. They were written clean-room, in two sessions, as the perl16 functions of 0.08 were:

- **Design session.** It wrote `docs/spec/setjmp.md`, `<setjmp.h>`, the `jmp_buf` layout and experiments in `docs/ABI.md`, and `tests/setjmp` (with its assembly checks, `sjasm.asm`), committed before any code. Its sources: C89 7.6; Microsoft's documentation of `<setjmp.h>`; libc16's ABI; and its own call-site experiments, compiling C that calls a `__cdecl` `setjmp` in each model and disassembling its own objects with `wdis`. Of `gr-dos/perl16` it read only `docs/M3-RUNTIME-AUDIT.md`. It read no Open Watcom runtime or other C library source.
- **Implementation session.** It worked in a copy of the repository without git history, told to read nothing outside it. It reported reading only the spec, the ABI, `crt/model.inc`, two existing assembly members for style, and the tests, and running the tools on libc16's own objects. It changed no existing member.

No Open Watcom runtime source or header was read and none of its libraries was disassembled.

### Running programs

`system`, the spawn family and `getpid` (0.10) are for `gr-dos/perl16` (#20, its audit's section 6.6); Tcl16 asked for `system` too (#12). They were written clean-room, in two sessions, as `setjmp` and `longjmp` were:

- **Design session.** It wrote `docs/spec/process.md`, `<process.h>`, `system` in `<stdlib.h>` and `tests/process` (with `child.c`, the program the tests run), committed before any code. Its sources: C89 7.10.4.5; Microsoft's documentation of `system` and the spawn functions; Ralf Brown's Interrupt List and MS-DOS 2.0 `SYSCALL.DOC` for INT 21h 4Bh, 4Dh, 48h, 49h and 43h; libc16's startup spec for the command-tail quoting; and its own experiments under DOSBox-X, running a test child with 4B00h through `intdosx` (see the spec's "Sources"). Of `gr-dos/perl16` it read only `docs/M3-RUNTIME-AUDIT.md`. It read no Open Watcom runtime or other C library source.
- **Implementation session.** It worked in a copy of the repository without git history, told to read nothing outside it. It reported reading the spec, the ABI, the specs of the startup, I/O and heap, `crt/model.inc`, a few existing members for style, and the tests, and running the tools on libc16's own objects. It changed no existing member, spec or test. It found that DOSBox-X runs its own command processor for 4B00h of any missing name ending in `COMMAND.COM`, so `system` checks that the command processor exists first; the design session added that to the spec.

No Open Watcom runtime source or header was read and none of its libraries was disassembled.

### Signals

`signal` and `raise` (0.11) are for `gr-dos/perl16` (#21, its audit's sections 6.6 and 7.4). They were written clean-room, in two sessions, as the spawn functions were:

- **Design session.** It wrote `docs/spec/signal.md`, `<signal.h>`, the change to `abort` in `docs/spec/stdlib.md`, and `tests/signal` (with its assembly helpers, `sigasm.asm`), committed before any code. Its sources: C89 7.7 and 7.10.4.1; Microsoft's documentation of `<signal.h>` for the numbers and values; Ralf Brown's Interrupt List and MS-DOS 2.0 `SYSCALL.DOC` for INT 23h and INT 21h 25h and 35h; libc16's ABI; and its own experiments under DOSBox-X, storing a Ctrl-C with INT 16h 05h and reading it with several DOS functions (see the spec's "Sources"). Of `gr-dos/perl16` it read only `docs/M3-RUNTIME-AUDIT.md`. It read no Open Watcom runtime or other C library source.
- **Implementation session.** It worked in a copy of the repository without git history, told to read nothing outside it. It reported reading the specs, the ABI, `crt/model.inc`, a few existing members for style (the communal variable of `rand` and `srand` among them), and the tests, and running the tools on libc16's own objects. It rewrote `abort` in assembly, since Open Watcom C cannot declare a communal variable, keeping its behaviour and message; it added `CPTR` to `crt/model.inc`. It changed no spec or test. It found that a handler at code offset 0 compared equal to `SIG_DFL`, and reserved those bytes in `sigtab`; the design session added that to the spec and to `docs/ABI.md`'s known issues.

No Open Watcom runtime source or header was read and none of its libraries was disassembled.

### The code reserve

The startup's code reserve (0.12, #27) moves `sigtab`'s 2-byte `BEGCODE` segment, which the 0.11 implementation session wrote, into `cstart`, for every small and compact program. The design session wrote the "Null function pointer" section of `docs/spec/startup.md`, the matching changes to `docs/spec/signal.md` and `docs/ABI.md`, `tests/startup/code0.c` and the code checks in `tests/startup/check-null`, committed before any code. A separate implementation session, in a copy of the repository without git history and told to read nothing outside it, added the segment to `crt/cstart.asm` and removed it from `crt/sigtab.asm`. It found by its own links that `option eliminate` dropped an unreferenced reserve, so `LIBC16_NULL`'s word is now the reserve's offset, which is 0, keeping it linked at no cost. It changed no spec or test. No Open Watcom runtime source or header was read and none of its libraries was disassembled.

## Similarity audit

`scripts/similarity-audit` (run by `make audit`, in CI and before each release; written for libc16, MIT) compares the code of every member of each library with the DOS runtime library of the same memory model in the pinned Open Watcom snapshot: `LIBC16.LIB` with `lib286/dos/clibs.lib` and `lib286/maths.lib`, `LIBC16C.LIB` with `clibc.lib` and `mathc.lib`, and `LIBC16L.LIB` with `clibl.lib` and `mathl.lib`, each with the objects in `lib286/dos`. It reads both as OMF records, takes the bytes of code-class segments, leaves out every byte a fixup patches, and cuts the rest into overlapping 8-byte runs. For each libc16 member it prints only the fraction of its runs found in the one Open Watcom member that shares most of them (*best*) and in the whole library (*any*). Nothing is disassembled, and no Open Watcom names, bytes or code are printed or kept. Before scoring, a control check requires every judged member to be found whole in the libc16 library itself. A member with at least 16 runs and a best score above 0.50 fails the audit. Members with fewer runs (a few instructions) are listed but not judged.

Results for 0.03: in `LIBC16.LIB` 83 of the 117 members are long enough to judge, and in `LIBC16C.LIB` and `LIBC16L.LIB` 88. The highest best score in any of them is 0.12 (`stricmp`, small); the next are `i4d` (0.11) and `int86` (0.10). The short members that score higher are those of 0.01 below, and in the compact and large libraries `intdos` (0.53, 15 runs) and `getc` (0.40, 5 runs): a call to `int86` with 21h, and a jump to `fgetc`. `intdosx`, a C call to `int86x` like `intdos` but with one more far pointer, failed the audit in the compact library (0.52, 21 runs) as the compiler's output for that call; it is now written in assembly (`crt/intdosx.asm`), with the small model's instructions unchanged and the far-data arguments copied in a loop, and scores 0.00.

Results for 0.04: the new members, `getch` (too short to judge) and `putch` (17 runs), score 0.00 in each library. 84 of the 119 members of `LIBC16.LIB` are judged, and 89 in `LIBC16C.LIB` and `LIBC16L.LIB`; the highest best score is still 0.12 (`stricmp`).

Results for 0.05: the new members score at most 0.03 (`rename`); `unlink` and `remove` score 0.00. 89 of the 124 members of `LIBC16.LIB` are judged, and 94 in `LIBC16C.LIB` and `LIBC16L.LIB`; the highest best score is still 0.12 (`stricmp`, small).

Results for 0.06: the new members score at most 0.07 (`strtol`, compact and large: a call to `strtoul` and its sign handling); `abort`, `chdir`, `clearerr` and `isascii` are too short to judge and score 0.00. 95 of the 134 members of `LIBC16.LIB` are judged, and 100 in `LIBC16C.LIB` and `LIBC16L.LIB`; the highest best score is still 0.12 (`stricmp`, small).

Results for 0.09: `setjmp` (34 to 36 runs) and `longjmp` (31 to 34) score 0.00 in every library. 107 of the 149 members of `LIBC16.LIB` are judged, and 112 in `LIBC16C.LIB` and `LIBC16L.LIB`; the highest best score is still `fromloc`'s 0.17.

Results for 0.11: `signal` (26 to 49 runs), `raise` (22 to 36) and `sigint` (30 to 38) score 0.00 in every library; `abort`, now in assembly, and `sigtab`, which has no code but its 2-byte reserve, are too short to judge. 114 of the 163 members of `LIBC16.LIB` are judged, and 121 in `LIBC16C.LIB` and `LIBC16L.LIB`; the highest best score is still 0.31 (`spawnve` and `spawnvpe`, compact and large).

Results for 0.10: `spawnx` (516 to 699 runs), `spawnp`, `system` and `dosexec` score at most 0.05 (`dosexec`, compact and large). `spawnve` and `spawnvpe` are judged only in the compact and large libraries, where they have 16 runs, the fewest judged, and score 0.31: each is a wrapper that pushes its four far arguments, and its try function, and calls `spawnx`, which compiled argument forwarding gives the same bytes in any runtime. They are now the highest best scores, ahead of `fromloc`'s 0.17. The other spawn wrappers and `getpid` are too short to judge. 111 of the 159 members of `LIBC16.LIB` are judged, and 118 in `LIBC16C.LIB` and `LIBC16L.LIB`.

Results for 0.08: the new members score at most 0.03 (`dup`, small); `atexit`, `exit`, `gmtime` and `srand` are too short to judge. `fromloc`, which is `__libc16_from_local` moved unchanged out of `timecal`, scores 0.12 (small) and 0.17 (compact and large) on its own: a short member (32 to 35 runs) of calendar arithmetic, which inside `timecal` had been diluted by the rest. It is now the library's highest best score, ahead of `stricmp` (0.12, small). 105 of the 147 members of `LIBC16.LIB` are judged, and 110 in `LIBC16C.LIB` and `LIBC16L.LIB`.

Results for the first release, with the snapshot of decision 1: 42 of the 65 members are long enough to judge. Their highest best score is 0.12 (`stricmp`); most are under 0.05. Four short members score higher: `i8ls` and `i8rs` (0.64, 11 runs), `u8rs` (0.73, 11 runs) and `putc` (1.00, 3 runs). They are a 64-bit shift loop and a jump to `fputc`, which have few possible encodings. The full report is `docs/AUDIT.txt` in each release archive.

## GR-DOS `compat/crt`

Not used. DESIGN.md §12 says it was written after examining Watcom runtime disassembly; GR-DOS's `docs/provenance.md` (its decision 20) says it was written from documented interfaces. The two disagree; this is to be resolved in GR-DOS during M3, when libc16 replaces it.

### GR-DOS's `compat/crt` (M3)

GR-DOS's ATTRIB and MEM linked its own runtime, `compat/crt`, written after the Open Watcom runtime then linked into them had been disassembled (DESIGN.md §12; GR-DOS `docs/provenance.md`, decision 20). libc16 replaces it and does not reuse it. The members those programs needed and libc16 lacked (`int86x`, `intdos`, `intdosx`, `segread`, `bdos`, `kbhit`, `exit`, and `struct SREGS`, `FP_SEG` and `FP_OFF`) were specified and written without reading `compat/crt`, `compat/include` or `compat/msc.h`. GR-DOS's documentation of `compat/crt` (its `docs/m4-utilities.md`) was read only after they were written, to update it. The call sites, in Microsoft's MS-DOS 4.00 sources for ATTRIB and MEM, were read.
