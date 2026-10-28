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
| `gr-dos/patch16` | `10c6d11` (GNU `patch` 2.2 trial build, its `docs/PORT-FEASIBILITY.md`) | **compact** | `open`, `read`, `write`, `close`, `lseek`, `stat`, `chmod`, `mkdir`, `getenv`, `fopen`, `fclose`, `fgetc`, `fputs`, `fwrite`, `fseek`, `ftell`, `rename`, `unlink`, `malloc`, `realloc`, `free`, string functions; its own formatting, temp files and signal handling |
| `tico-edit/tico16` | `5769a9a6bc9d` | **compact** | `malloc`, `realloc`, `free`, `int86`, `intdosx`, `segread`, `_dos_*` file and vector functions, `_harderr`, `_fmemcpy`, `memmove`, `sprintf` and others |

GR-DOS's ATTRIB, MEM and FDISK are Microsoft's MS-DOS 4.00 sources (Microsoft C 5.1), as GR-DOS builds them with Open Watcom C, listed from the call sites rather than an Open Watcom build. Every other program also references `_cstart_`, `__argc` (all of them take `argc`/`argv`) and `_small_code_` or `_big_code_`, a model marker that `-zl` leaves out.

## Observations

- **Memory models.** `tar16` (large) and `tico16` (compact) could not link with libc16 0.02, which had only the small model. From 0.03 libc16 has a library per model (decision 11): `LIBC16C.LIB` for `-mc` and `LIBC16L.LIB` for `-ml`, and both now link it ("Migration status"). Their functions are `__cdecl` with far data pointers as well, so `tico16`'s `_harderr` handler is declared `__cdecl __far` (`docs/spec/dos.md`), and `tar16` defines the POSIX names it uses (`S_IRUSR`, `S_ISDIR` and the like) from Microsoft's `S_IREAD`, `S_IWRITE` and `S_IFMT`, which are what libc16's `<sys/stat.h>` has. Open Watcom's `<stdio.h>` makes `fileno` a macro, so it was missing from the inventory; libc16 0.03 has it as a macro too.
- **Formatting.** Every consumer uses `printf` and `fprintf`, and `df16`, `which16`, `tar16` and `tico16` use `sprintf`. Decision 3 prefers `libc16_` helpers for simple formatting, so each migration includes replacing the simple calls in the consumer.
- **Streams.** Every consumer writes to `stdout`/`stderr` through `FILE`; `crcsum16` and `md5sum16` also read files through `fopen`/`fgetc`.
- **errno.** Open Watcom's headers reach `errno` through `__get_errno_ptr`; libc16's headers define their own.
- **64-bit.** Only `df16` needs 64-bit helpers so far (`__U8M`, `__U8DQ`, `__U8DR`).

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

Where a consumer has an assembly version that its tests compare byte for byte with the C program, the assembly had copied Open Watcom's runtime behaviour. It now follows libc16's specs instead: Microsoft's argument splitting, libc16's `errno` mapping and messages, unbuffered streams, `_fullpath` and `getenv` limits.

crcsum16 found the `DGROUP:0` bug fixed in `77b402e`: its first string was at offset 0 and so compared equal to `NULL`.
