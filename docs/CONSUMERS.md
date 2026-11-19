# libc16 Consumers

Inventory of what each candidate consumer needs from a C runtime (DESIGN.md §7–8), taken on 2026-10-26. Each repository's C sources, at the revision shown, were compiled with its own flags and Open Watcom's headers; the list is the external symbols of the objects that the program does not define itself. Symbols are Open Watcom's register-convention names (`strcmp_`); built against libc16's headers, the public functions become `__cdecl` (`_strcmp`).

| Repository | Revision | Model | Needs |
|---|---|---|---|
| `gr-dos/hello16` | `2904d6936366` | small | `fprintf`, `printf`, `strcmp`, `__iob` (stdio streams) |
| `gr-dos/truncate16` | `d2708f8a36cd` | small | `open`, `close`, `chsize`, `filelength`, `strerror`, `errno`, `fprintf`, `printf`, `strchr`, `strcmp`, `strncmp`; helpers `__I4D`, `__I4M`, `__U4D`, `__U4M` |
| `gr-dos/crcsum16` | `b20a6d1cfd6b` | small | `open`, `read`, `close`, `fopen`, `fclose`, `fgetc`, `ungetc`, `fflush`, `fprintf`, `printf`, `_dos_findfirst`, `_dos_findnext`, `memcpy`, `strchr`, `strcpy`, `strlen`, `stricmp`, `strerror`, `errno` |
| `gr-dos/df16` | `bc1bc4a35ff6` | small | `int86`, `fflush`, `fprintf`, `printf`, `sprintf`, `strcat`, `strchr`, `strcpy`, `strlen`; helpers `__U8DQ`, `__U8DR`, `__U8M` |
| `gr-dos/md5sum16` | `9a2ac49b9ea7` | small | `open`, `read`, `close`, `access`, `setmode`, `fopen`, `fclose`, `fgetc`, `fgets`, `fputc`, `fputs`, `fprintf`, `printf`, `_dos_findfirst`, `_dos_findnext`, `memcmp`, `memcpy`, `memset`, `strchr`, `strcmp`, `strcpy`, `strlen`, `strlwr`, `strncmp`, `strerror`, `errno` |
| `gr-dos/which16` | `8358fa1ccfe9` | small | `getenv`, `_fullpath`, `_dos_findfirst`, `fprintf`, `printf`, `sprintf`, `memcpy`, `strcat`, `strchr`, `strcmp`, `strdup`, `stricmp`, `strlen`, `strlwr`, `strrchr` |
| `gr-dos/tar16` | `ac2c8c4e928e` | **large** | about 45 functions, including `malloc`, `calloc`, `free`, `stat`, `utime`, `localtime`, `strftime`, `tzset`, `mkdir`, `chmod` |
| `gr-dos/gr-dos` ATTRIB | `5b94dff6` | small, own entry (ATTRIBA) | `bdos`, `close`, `filelength`, `int86`, `intdos`, `intdosx`, `segread`, `strcat`, `strcmp`, `strcpy`, `strlen`; no `_cstart_` |
| `gr-dos/gr-dos` MEM | `5b94dff6` | small | `exit`, `int86`, `int86x`, `intdos`, `intdosx`, `kbhit`, `segread`, `strcpy`, `_psp` |
| `gr-dos/gr-dos` FDISK | `978f93d2` | small | `bdos`, `exit`, `getch`, `putch`, `int86`, `int86x`, `intdos`, `intdosx`, `segread`, `_dos_setvect`, `strcat`, `strchr`, `strcmp`, `strcpy`, `strlen`, `strncpy`, `memset`, `isalpha`, `isdigit`, `libc16_ltoa` |
| `tico-edit/tico16` | `5769a9a6bc9d` | **compact** | `malloc`, `realloc`, `free`, `int86`, `intdosx`, `segread`, `_dos_*` file and vector functions, `_harderr`, `_fmemcpy`, `memmove`, `sprintf` and others |

GR-DOS's ATTRIB, MEM and FDISK are Microsoft's MS-DOS 4.00 sources (Microsoft C 5.1), as GR-DOS builds them with Open Watcom C, listed from the call sites rather than an Open Watcom build. Every other program also references `_cstart_`, `__argc` (all of them take `argc`/`argv`) and `_small_code_` or `_big_code_`, a model marker that `-zl` leaves out.

## Observations

- **Memory models.** `tar16` (large) and `tico16` (compact) could not link with libc16 0.02, which had only the small model. From 0.03 libc16 has a library per model (decision 11): `LIBC16C.LIB` for `-mc` and `LIBC16L.LIB` for `-ml`, and both now link it ("Migration status"). Their functions are `__cdecl` with far data pointers as well, so `tico16`'s `_harderr` handler is declared `__cdecl __far` (`docs/spec/dos.md`), and `tar16` defines the POSIX names it uses (`S_IRUSR`, `S_ISDIR` and the like) from Microsoft's `S_IREAD`, `S_IWRITE` and `S_IFMT`, which are what libc16's `<sys/stat.h>` has. Open Watcom's `<stdio.h>` makes `fileno` a macro, so it was missing from the inventory; libc16 0.03 has it as a macro too.
- **Formatting.** Every consumer uses `printf` and `fprintf`, and `df16`, `which16`, `tar16` and `tico16` use `sprintf`. Decision 3 prefers `libc16_` helpers for simple formatting, so each migration includes replacing the simple calls in the consumer.
- **Streams.** Every consumer writes to `stdout`/`stderr` through `FILE`; `crcsum16` and `md5sum16` also read files through `fopen`/`fgetc`.
- **errno.** Open Watcom's headers reach `errno` through `__get_errno_ptr`; libc16's headers define their own.
- **64-bit.** Only `df16` needs 64-bit helpers so far (`__U8M`, `__U8DQ`, `__U8DR`).

## Tcl16 (#12)

`gr-dos/tcl16` ports Tcl 6.7 (and Tiny Tcl) in the large model. Its M1 and M2 reports list what linking it with libc16 0.04 left undefined. As of 0.05, the gaps were:

| Function | In 0.05 | 0.06 |
|---|---|---|
| `fseek`, `ftell` | yes (`docs/spec/stdio.md`, `tests/stdio/seek`) | |
| `strtol`, `strcspn`, `strstr`, `qsort`, `abort`, `clearerr` | no | added |
| `getcwd`, `chdir`, `fstat`, `isascii` | no | added |
| `environ` | no | deferred |
| `opendir`, `readdir`, `closedir` | no | deferred: Tcl16-local adapter |
| `strtod`, `sscanf`, `sprintf`, the `printf` family | no | out of scope (decision 3; no floating point) |
| `system`, `_mktemp` | no | out of scope (#12) |

Sizes of the new members in bytes of code, from `wdis` of each object. A program pays only for the members it calls:

| Member | Small | Compact | Large | Also links |
|---|---|---|---|---|
| `strtol` | 158 | 188 | 190 | `strtoul`, `errno` |
| `strcspn` | 39 | 43 | 43 | |
| `strstr` | 55 | 61 | 61 | |
| `qsort` | 222 | 258 | 259 | nothing |
| `abort` | 18 + 31 data | 20 + 31 | 20 + 31 | nothing |
| `clearerr` | 12 | 15 | 15 | nothing (the macro links nothing at all) |
| `isascii` | 19 | 19 | 19 | |
| `fstat` | 296 | 299 | 303 | `dostime`, the time zone members and `getenv`, as `stat` does |
| `getcwd` | 107 | 124 | 126 | `errno`, `doserr` |
| `chdir` | 25 | 25 | 27 | `errno`, `doserr` |

Decisions on the DOS and POSIX extensions:

- **`getcwd`, `chdir`:** included. They are Microsoft C's, small, and each is one or two DOS calls. `getcwd` with a null buffer fails with `EINVAL`, where Microsoft's allocates. That keeps the heap out of every program that calls it.
- **`fstat`:** included. It is Microsoft C's, and the same size and dependencies as `stat`: it links the time zone members for its times.
- **`isascii`:** included as a function like the other `<ctype.h>` members, which have no macros (`docs/spec/ctype.md`). It is a few bytes, and costs nothing unless called.
- **`environ`:** deferred. A `char **environ` has to be built before `main`, from an init routine (decision 8), into memory that grows with the environment, so from the heap. Its layout would become ABI, and a program that changes it would expect `getenv` to see the change, which `getenv` (reading the PSP's environment) does not. Tcl16 needs only to enumerate the environment for its `env` array. A small, documented enumerator (for example `libc16_envnext`, over the PSP's environment block, needing no heap) is the likely shape, decided with Tcl16's M3.
- **`opendir`, `readdir`, `closedir`:** deferred. Tcl16's M1 report already proposes an adapter over `_dos_findfirst`/`_dos_findnext`, local to Tcl16. A libc16 `DIR` would need the heap or a fixed table, and POSIX's `d_name` and `rewinddir` add little for DOS programs. It can be revisited if a second consumer needs it.

## Perl16 (#22)

`gr-dos/perl16` ports Perl 3 in the large model. Its M3 runtime audit (`docs/M3-RUNTIME-AUDIT.md`, sections 6.2 to 6.7) assigns these small C89 and Microsoft C functions to libc16, and libc16 0.08 adds them. `system` and the spawn family, and `signal`/`raise` are separate issues; floating point, formatting and `environ` are perl16's or `libkitchen16`'s (that audit, section 11).

Sizes in bytes of code, from `wdis` of each object. A program pays only for the members it calls:

| Member | Small | Compact | Large | Also links, or data |
|---|---|---|---|---|
| `atexit` | 49 | 51 | 59 | a termination-table entry (2 / 2 / 4) and 66 / 66 / 130 bytes of BSS |
| `rand` | 47 | 47 | 47 | 4 bytes of BSS, shared with `srand`; no 32-bit helper |
| `srand` | 17 | 17 | 17 | the same 4 bytes |
| `time` | 263 | 281 | 293 | `timecal`, `tzset` and `getenv`, so the startup |
| `gmtime` | 32 | 39 | 41 | `timecal` (not `tzset`); 18 bytes of BSS |
| `isatty` | 38 | 39 | 39 | `errno` |
| `dup` | 73 | 76 | 78 | `fdtab`, `errno`, `doserr` |
| `dup2` | 87 | 90 | 92 | `fdtab`, `errno`, `doserr` |
| `fdopen` | 148 | 183 | 185 | `streams` (56 / 87 / 87, with the 700 / 720 bytes of BSS that `fopen` had); not `fopen` or `open` |
| `rmdir` | 54 | 55 | 57 | `errno`, `doserr` |
| `strupr` | 31 | 34 | 34 | |

0.09 adds `setjmp` and `longjmp` (#19, the audit's sections 6.3 and 7.3). `jmp_buf` is 8 words in every model (`docs/ABI.md`). Each is its own member and links nothing else:

| Member | Small | Compact | Large |
|---|---|---|---|
| `setjmp` | 43 | 41 | 42 |
| `longjmp` | 38 | 40 | 41 |

0.10 adds `system`, `spawnl`, `spawnv`, `spawnve`, `spawnvp`, `spawnvpe` and `getpid` (#20, the audit's section 6.6), in `<process.h>` (`docs/spec/process.md`). Only `P_WAIT` is supported. The tail is quoted so that a libc16 child gets back the `argv` it was given; the `e` forms build the environment from `envp` alone, in a block from DOS rather than the heap. A child gets the memory above the heap's highest point: the heap never shrinks the program's block. Under DOSBox-X, `system` returns `COMMAND.COM`'s exit code, which is 1 whenever it ran a program, not that program's code; perl16 should use the spawn functions where it needs an exit status.

| Member | Small | Compact | Large | Also links, or data |
|---|---|---|---|---|
| `spawnl`, `spawnv`, `spawnvp` | 23 | 30 / 31 / 31 | 36 / 37 / 37 | `spawnx` |
| `spawnve`, `spawnvpe` | 24 | 33 | 39 | `spawnx` |
| `spawnx` (private) | 591 | 789 | 798 | `dosexec`, `doserr`, `errno` |
| `spawnp` (private, the `p` forms) | 201 | 264 | 270 | `getenv`, so the startup; 5 bytes of constants |
| `dosexec` (private) | 99 | 101 | 101 | 2 bytes of BSS |
| `system` | 167 | 214 | 220 | `dosexec`, `getenv`, `doserr`, `errno`; 21 bytes of constants |
| `getpid` | 4 | 5 | 5 | the startup |

The spawn functions use about 260 bytes of stack, and the `p` forms 128 more; `system` uses about 130.

0.11 adds `signal` and `raise` (#21, the audit's sections 6.6 and 7.4), in `<signal.h>` (`docs/spec/signal.md`), with Microsoft C's signal numbers. Every number from 1 to 31 can be set and raised; only `SIGINT` (Ctrl-C, through INT 23h) and `SIGABRT` (`abort`) arrive otherwise. `SIGINT` arrives only during a DOS call that checks for Ctrl-C, and a handler may call DOS, `longjmp` out or `exit`. `SIG_DFL` from `raise` ends the program with exit code 3. A Ctrl-C while a child runs goes to DOS's handler, not perl16's.

| Member | Small | Compact | Large | Also links, or data |
|---|---|---|---|---|
| `signal` | 83 | 83 | 122 | `sigtab`, `sigint`, `errno` |
| `raise` | 55 | 55 | 79 | `sigtab`, `errno` |
| `sigint` (private) | 119 | 119 | 138 | 5 bytes of BSS |
| `sigtab` (private) | 0 | 0 | 0 | 62 / 62 / 124 bytes of BSS (until 0.12 it also had a 2-byte code reserve in the small and compact models, now the startup's) |
| `abort` | 27 | 29 | 33 | was 18 / 20 / 20; now in assembly, with the communal `___libc16_sigabrt` (2 / 2 / 4 bytes of BSS) |

0.12 moves that code reserve to the startup (#27, `docs/spec/startup.md`, "Null function pointer"): in the small and compact models `cstart` puts 2 bytes, `LIBC16_CODE0`, before all code, so that no function is at offset 0 or 1 and no function pointer compares equal to a null pointer, `SIG_DFL` or `SIG_IGN`, whether or not the program links `signal`. `cstart`'s code is unchanged; the reserve adds 2 bytes to every small and compact program, which usually fit in the padding before the paragraph-aligned `DGROUP` (`tests/startup/status` stays 162 bytes) and otherwise cost one paragraph. The large model is unchanged. A program with its own entry point gets no reserve.

0.16 adds buffered stream output (#34, `docs/spec/stdio.md`, "Buffering"): C89's `setvbuf` and `setbuf`, with `_IOFBF`, `_IOLBF` and `_IONBF`. Every stream still starts unbuffered, so a program that does not call them writes, and links, as before, but for `bufhook`'s two pointers. perl16's clean-runtime build was 51.5% slower than its Open Watcom build printing 2,000 lines of four items each (#34), about 8,000 DOS writes; with a buffer that workload makes 59 (`tests/bufperf`). perl16 should ask for buffers itself, at startup and for each file it opens for writing:

    setvbuf(stdout, NULL, isatty(1) ? _IOLBF : _IOFBF, BUFSIZ);
    /* after each fopen or fdopen for writing */
    setvbuf(fp, NULL, _IOFBF, BUFSIZ);

and leave `stderr` unbuffered. The buffers come from the far heap in the large model, not from `DGROUP`; the engine adds 41 bytes to `DGROUP` (37 compact, 21 small), and `bufhook` 8 (4). A buffered stream's output is written when the buffer fills, by `fflush`, `fclose`, `fseek`, `setvbuf`, at the end of a line for a line-buffered stream, before any input refill for line-buffered streams, before `system`, `libc16_systeme` and the spawn functions run a child (which they do not run, returning -1, if that write fails), and by `exit` or returning from `main`; not by `abort`, a signal's default action or a direct DOS exit. Code that writes to a stream's handle with `write`, or `dup2`s onto it, must `fflush` the stream first. A failed or short write sets the error indicator and `errno` (`ENOSPC` for a full disk), fails the call that made it, and keeps earlier calls' unwritten output pending for a later `fflush`. `FILE` keeps its size and layout, so `gr-dos/libkitchen16`'s `printf` family, which writes through `fwrite`, is buffered with no change.

Sizes in bytes of code, from `wdis` of each object, and data:

| Member | Small | Compact | Large | Also links, or data |
|---|---|---|---|---|
| `setvbuf` | 101 | 123 | 133 | `bufio`, `malloc`, `free` |
| `setbuf` | 29 | 47 | 50 | `bufio`; no heap |
| `bufio` (private, the engine) | 1105 | 1484 | 1481 | `bufexit`, `bufhook`, `memcpy`, `fdtab`, `doserr`, `errno`; 18 / 34 / 36 bytes of BSS |
| `bufexit` (private) | 10 | 12 | 14 | a termination-table entry (2 / 2 / 4) and 1 byte of BSS |
| `bufhook` (private) | 0 | 0 | 0 | 4 / 4 / 8 bytes of BSS |
| `fputc` | 115 | 152 | 154 | was 61 / 70 / 72; a buffered byte that is not `\n` goes in without a call |
| `fputs` | 79 | 100 | 104 | was 57 / 73 / 77 |
| `fwrite` | 156 | 182 | 184 | was 136 / 157 / 159 |
| `fflush` | 32 | 43 | 56 | was 7 / 7 / 7 |
| `fclose` | 69 | 83 | 85 | was 35 / 43 / 45 |
| `fseek` | 283 | 303 | 311 | was 252 / 268 / 277 |
| `ftell` | 423 | 464 | 469 | was 414 / 442 / 447; links nothing new |
| `fgetc` | 135 | 176 | 187 | was 118 / 151 / 153 |
| `fread` | 261 | 311 | 324 | was 225 / 268 / 272 |
| `system` | 267 | 332 | 358 | was 231 / 295 / 311 |
| `spawnx` (private) | 513 | 674 | 693 | was 480 / 643 / 651 |

0.15 adds `libc16_systeme(cmd, envp)` to `<process.h>` (#31, `docs/spec/process.md`): `system` with an environment. `cmd` reaches the command processor byte for byte after ` /C `, as with `system`, so quotes, backslashes and redirection are the shell's to interpret; the child's environment is built from `envp` as `spawnve` builds it, and a null `envp` inherits, as `system` does. The command processor is still the parent's `COMSPEC`. perl16 can run every shell command with its `%ENV` this way, instead of falling back to `system` for commands with double quotes:

    char *shell_env[N];         /* "NAME=value" strings from %ENV, then NULL */
    int status = libc16_systeme("find \"x y\" <in >out", shell_env);
    if (status == -1)
        /* errno: ENOENT (no command processor), E2BIG (command over
         * 122 bytes, or environment over 32767), ENOMEM */;

The environment block is moved out of `spawnx` into its own private member, `envblock`, which `spawnve` and `spawnvpe` link (and `spawnl`, `spawnv` and `spawnvp`, through `spawnx`); their code is the same in total. `system`'s member now holds the core it shares with `libc16_systeme`, which adds 64 bytes to a small-model program that calls `system` (81 compact, 91 large).

| Member | Small | Compact | Large | Also links, or data |
|---|---|---|---|---|
| `libc16_systeme` | 17 | 24 | 30 | `system`, `envblock`, `getenv`, `dosexec`, `doserr`, `errno` |
| `system` | 231 | 295 | 311 | was 167 / 214 / 220 |
| `envblock` (private) | 132 | 176 | 176 | new; was part of `spawnx` |
| `spawnx` (private) | 480 | 643 | 651 | was 591 / 789 / 798 |

0.14 makes the heap's time independent of the number of blocks (#30, `docs/spec/heap.md`, "Performance"), for perl16's Perl 3, whose hash and recursion tests took minutes with 0.12: `malloc` keeps free blocks on size-class lists instead of searching every block, `free` merges neighbours at once, and `realloc` grows a block in place into a free block after it or at the end of the heap. The interface is unchanged. Blocks are now multiples of 4 bytes, at least 8 (12 with far data), so a program of many tiny blocks needs a little more memory. There is no 0.13.

| Member | Small | Compact | Large | Also links, or data |
|---|---|---|---|---|
| `malloc` | 386 | 658 | 670 | was 189 / 405 / 405; 2 / 4 / 4 bytes of BSS |
| `free` | 238 | 335 | 338 | was 16 / 54 / 54; the free lists, 142 / 204 / 204 bytes of BSS |
| `realloc` | 222 | 318 | 333 | was 120 / 162 / 172 |

Changes to existing members in 0.08:

- **`cstart`** grows by 16 bytes in every model (83 / 90 / 93 → 99 / 106 / 109), and **`exit`** by 16 to 18 (10 → 26 / 27 / 28), for walking the termination table. A program that does not call `atexit` links no further member and no data; the table is empty.
- **`fopen`** moved its stream table and buffers to `streams`. Together they are the same size in the small model (176 bytes) and 30 or 32 bytes more in the compact and large.
- **`timecal`** gave `__libc16_from_local` to a new member, `fromloc` (97 / 108 / 114), which `stat`, `fstat` and `utime` link through `dostime`; together they are 4 to 6 bytes more.

Decisions:

- **`atexit`** uses a termination table like the init table (DESIGN.md decision 8), so `exit` references no `atexit` member. 32 functions, the least C89 allows, in BSS.
- **`rand`** returns the high 15 bits of a 32-bit LCG (1664525, 1013904223), not C89's example generator, as perl16's audit (section 7.10) asks. perl16 should set `RANDBITS` to 15.
- **`dup2`** returns 0, as Microsoft C's does, not `fd2` as POSIX's does; callers that test for -1 work with either.
- **`fdopen`** without `b` or `t` keeps the handle's mode (Microsoft's sets text), so that `fdopen(dup(fd), ...)` reads and writes as `fd` does.

## Using libc16

A consumer gets libc16 from a release (DESIGN.md §3 and §9):

- **`scripts/fetch-libc16`**, a copy of libc16's `consumers/fetch-libc16`, pins one release by `version` and `sha256`. It downloads `libc16-<version>.zip` from the `v<version>` release, checks its SHA-256 before unpacking it, and unpacks its `libc16-<version>/` directory as `build/libc16`. CI and releases never follow a branch.
- **`LIBC16_ROOT`** (default `build/libc16`) is the libc16 tree the `Makefile` uses. It may also name an unpacked release elsewhere, or a libc16 checkout built with `make`, whose `build/stage` has the same layout. That is the local override for development and A/B comparisons. The `Makefile` reads `$(LIBC16_DIR)/lib/LIBC16.LIB` (or `LIBC16C.LIB`, `LIBC16L.LIB` for a compact or large program) and `$(LIBC16_DIR)/include`, where `LIBC16_DIR` is `$(LIBC16_ROOT)/build/stage` if that exists and `$(LIBC16_ROOT)` otherwise.
- **The toolchain** is the Open Watcom snapshot the release was built with. `$(LIBC16_DIR)/tools/fetch-watcom build/tools` downloads it, checks its SHA-256 and prints the `WATCOM` directory. CI caches `build/tools` keyed on `scripts/fetch-libc16`.
- **`LIBC16_URL`** makes `scripts/fetch-libc16` fetch the same archive from elsewhere, such as a mirror or a candidate build being tested. The pinned SHA-256 still applies.

hello16's `Makefile` is the reference for these rules.

**Updating a consumer** to a new libc16 is a deliberate change of its own: set `version` and `sha256` in `scripts/fetch-libc16` to the new release's values (the `.sha256` asset, also in the release notes), in the same commit, and run its tests and any assembly comparisons. Copy a newer `consumers/fetch-libc16` only when the script itself changes.

## Migration status

Each small-model consumer has a PR that links it with libc16 instead of Open Watcom's runtime. Those PRs first pinned libc16 commit `8cde3bd` and built it from source, a temporary bootstrap until the first release. They now pin release v0.01 by version and SHA-256 (`Using libc16`), and CI passes on each. `tar16` and `tico16`, which need the large and compact models, pin release v0.03, the first with those libraries. Sizes are the `.EXE` before and after; the C sources change only where libc16 leaves out a function (`printf`, `fprintf`, `sprintf`).

| Repository | PR | `.EXE` bytes, Open Watcom → libc16 | Assembly `.COM` |
|---|---|---|---|
| `gr-dos/hello16` | [#4](https://github.com/gr-dos/hello16/pull/4) | 9,532 → 1,386 | 547 bytes, updated to follow libc16 |
| `gr-dos/truncate16` | [#4](https://github.com/gr-dos/truncate16/pull/4) | 16,184 → 5,728 | 3,294 bytes, updated |
| `gr-dos/crcsum16` | [#5](https://github.com/gr-dos/crcsum16/pull/5) | 17,042 → 5,948 | 3,417 bytes, updated |
| `gr-dos/df16` | [#5](https://github.com/gr-dos/df16/pull/5) | 14,938 → 4,680 | none |
| `gr-dos/md5sum16` | [#2](https://github.com/gr-dos/md5sum16/pull/2) | 25,610 → 14,302 | none |
| `gr-dos/which16` | [#8](https://github.com/gr-dos/which16/pull/8) | 13,220 → 4,252 | 2,530 bytes, updated |
| `gr-dos/tar16` | [#4](https://github.com/gr-dos/tar16/pull/4) | 43,296 → 20,758 (large) | none |
| `tico-edit/tico16` | [#3](https://github.com/tico-edit/tico16/pull/3) | 29,702 → 18,378 (compact) | none |

`tar16` and `tico16` also change where libc16's interfaces differ ("Observations"). DOSBox-X never raises INT 24h, so `tico16`'s test build raises it itself (a `{Crit}` key) to check that its `_harderr` handler is called with DOS's arguments and fails the call.

GR-DOS (M3, DESIGN.md §8) replaces its own runtime, `compat/crt`, with libc16 0.02 for ATTRIB and MEM: [gr-dos/gr-dos#9](https://github.com/gr-dos/gr-dos/pull/9). ATTRIB keeps Microsoft's entry code, ATTRIBA, and links no libc16 startup.

| Program | `compat/crt` → libc16 | 360 KB disk clusters | Own memory (`MEM /PROGRAM`) |
|---|---|---|---|
| `ATTRIB.EXE` | 19,446 → 19,369 bytes | 19 → 19 | |
| `MEM.EXE` | 11,986 → 12,036 bytes | 12 → 12 | 3A40h → 3A70h bytes |

GR-DOS's FDISK ([gr-dos/gr-dos#11](https://github.com/gr-dos/gr-dos/pull/11)) links libc16 0.03, with `getch` and `putch` as macros over `bdos` and `sprintf` replaced by two helpers built on `libc16_ltoa`; 37,694 bytes, where Microsoft's EXEPACKed build is 60,935. libc16 0.04 adds `getch` and `putch` so that the macros can go.

`gr-dos/patch16`, a port of GNU `patch` 2.2, needs `rename`, `unlink`, `remove`, `fseek` and `ftell` besides what libc16 already had; libc16 0.05 adds them.

Where a consumer has an assembly version that its tests compare byte for byte with the C program, the assembly had copied Open Watcom's runtime behaviour. It now follows libc16's specs instead: Microsoft's argument splitting, libc16's `errno` mapping and messages, unbuffered streams, `_fullpath` and `getenv` limits.

crcsum16 found the `DGROUP:0` bug fixed in `77b402e`: its first string was at offset 0 and so compared equal to `NULL`.
