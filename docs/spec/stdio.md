# Spec: stdio streams

**Header:** `<stdio.h>`  
**Members:** one per stream object (`stdin`, `stdout`, `stderr`) and one per function  
**Tests:** `tests/stdio`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- C89 7.9 (input/output), for every function listed here.
- `docs/spec/io.md`, which streams are built on.

## Scope

libc16 implements the stream functions its consumers use (`docs/CONSUMERS.md`) with their C89 semantics, and no others: `fopen`, `fclose`, `fflush`, `fgetc`, `fgets`, `fputc`, `fputs`, `ungetc`, `feof`, `ferror`. The formatted functions (`printf` and the rest) are not provided (DESIGN.md §7; decision 3).

## Streams

- `stdin`, `stdout` and `stderr` are handles 0, 1 and 2, in the handle's mode (text unless changed with `setmode`). Each is its own member, so a program that uses only `stderr` links only `stderr`.
- **Output is unbuffered:** every output call reaches DOS (`write`) before it returns. Nothing is lost when a program ends without closing or flushing, and output to `stdout` and `stderr` appears in the order it was written. (C89 7.9.3 allows any stream to be unbuffered.)
- **Input is buffered**, from `read`, so text-mode translation is `read`'s. `stdin`'s buffer is 128 bytes; a stream from `fopen` has its own.
- `fopen` streams use a fixed table, so that `fopen` needs no heap: with the standard streams, `FOPEN_MAX` (8) streams can be open at once. A further `fopen` fails with `EMFILE`.

## Functions

- **`FILE *fopen(const char *path, const char *mode)`:** `mode` is `r`, `w` or `a`, optionally followed by `b` (binary; otherwise text). `w` creates or truncates, `a` creates and appends (`O_APPEND`). On failure returns a null pointer with `errno` from `open`. The update modes (`+`) are not supported and fail with `EINVAL`, as does any other mode.
- **`int fclose(FILE *fp)`:** closes the stream and its handle; 0, or `EOF` if `close` fails. The standard streams may be closed too.
- **`int fflush(FILE *fp)`:** 0. Output is unbuffered, so there is nothing to write; `fflush(NULL)` is also 0. On an input stream it does nothing (C leaves that undefined).
- **`int fgetc(FILE *fp)`:** the next byte as an `unsigned char` converted to `int`, or `EOF` with the end-of-file indicator set at end of file, or with the error indicator set on a read error.
- **`char *fgets(char *s, int n, FILE *fp)`:** reads at most `n - 1` bytes, stopping after a `\n`, and stores a terminating null. Returns `s`, or a null pointer if end of file came before any byte was read (leaving `s` unchanged) or a read error occurred.
- **`int fputc(int c, FILE *fp)`:** writes `c` as an `unsigned char`; returns it, or `EOF` with the error indicator set.
- **`int fputs(const char *s, FILE *fp)`:** writes `s` without its terminator; returns 0, or `EOF` with the error indicator set.
- **`int ungetc(int c, FILE *fp)`:** pushes back one byte, which the next read returns; clears the end-of-file indicator. Pushing `EOF`, or a second byte before the first is read, fails with `EOF`.
- **`int feof(FILE *fp)`, `int ferror(FILE *fp)`:** the indicators; also macros.

Reading from a stream opened for writing, or writing to one opened for reading, fails with the error indicator set.

## Constraints

All are `__cdecl`. 8086 instructions only.
