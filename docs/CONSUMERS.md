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
| `tico-edit/tico16` | `5769a9a6bc9d` | **compact** | `malloc`, `realloc`, `free`, `int86`, `intdosx`, `segread`, `_dos_*` file and vector functions, `_harderr`, `_fmemcpy`, `memmove`, `sprintf` and others |

Every program also references `_cstart_`, `__argc` (all of them take `argc`/`argv`) and `_small_code_` or `_big_code_`, a model marker that `-zl` leaves out.

## Observations

- **Memory models.** `tar16` (large) and `tico16` (compact) cannot link with libc16, which supports only the small model (DESIGN.md §2). They stay `pending` unless they move to the small model or libc16 adds a model.
- **Formatting.** Every consumer uses `printf` and `fprintf`, and `df16`, `which16`, `tar16` and `tico16` use `sprintf`. Decision 3 prefers `libc16_` helpers for simple formatting, so each migration includes replacing the simple calls in the consumer.
- **Streams.** Every consumer writes to `stdout`/`stderr` through `FILE`; `crcsum16` and `md5sum16` also read files through `fopen`/`fgetc`.
- **errno.** Open Watcom's headers reach `errno` through `__get_errno_ptr`; libc16's headers define their own.
- **64-bit.** Only `df16` needs 64-bit helpers so far (`__U8M`, `__U8DQ`, `__U8DR`).
