# Spec: stdio streams

**Header:** `<stdio.h>`  
**Members:** one per stream object (`stdin`, `stdout`, `stderr`) and one per function  
**Tests:** `tests/stdio`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- C89 7.9 (input/output), for every function listed here; 7.9.9.2 and 7.9.9.4 for `fseek` and `ftell`.
- `docs/spec/io.md`, which streams are built on.

## Scope

libc16 implements the stream functions its consumers use (`docs/CONSUMERS.md`) with their C89 semantics, and no others: `fopen`, `fclose`, `fflush`, `fgetc`, `getc`, `fgets`, `fread`, `fputc`, `putc`, `putchar`, `fputs`, `fwrite`, `ungetc`, `fseek`, `ftell`, `feof`, `ferror`, and Microsoft's `fileno`. `remove` and `rename` are declared in `<stdio.h>` too and specified in `docs/spec/files.md`. The formatted functions (`printf` and the rest) are not provided (DESIGN.md §7; decision 3).

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
- **`size_t fread(void *ptr, size_t size, size_t nmemb, FILE *fp)`:** reads up to `nmemb` elements of `size` bytes into `ptr`, as if by `fgetc` for each byte (a pushed-back byte first), and returns the number of complete elements read: fewer than `nmemb` only at end of file, with the end-of-file indicator set, or on a read error, with the error indicator set. A partly read element's bytes are stored but not counted. If `size` or `nmemb` is 0, returns 0 and reads nothing.
- **`size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *fp)`:** writes `nmemb` elements of `size` bytes from `ptr`, as if by `fputc` for each byte, and returns the number of complete elements written; fewer than `nmemb` only on a write error (including a full disk), with the error indicator set. If `size` or `nmemb` is 0, returns 0 and writes nothing.
- **`int fputc(int c, FILE *fp)`:** writes `c` as an `unsigned char`; returns it, or `EOF` with the error indicator set.
- **`int fputs(const char *s, FILE *fp)`:** writes `s` without its terminator; returns 0, or `EOF` with the error indicator set.
- **`int ungetc(int c, FILE *fp)`:** pushes back one byte, which the next read returns; clears the end-of-file indicator. Pushing `EOF`, or a second byte before the first is read, fails with `EOF`.
- **`long ftell(FILE *fp)`:** the stream's file position: the offset in bytes from the start of the file of the next byte a read would return, counting the bytes stored in the file (so in text mode a `\r\n` read as `\n` counts 2). Bytes read into the stream's buffer and not yet returned, and a byte pushed back by `ungetc`, are before it: a pushed-back byte counts as the byte that preceded the buffered ones in the file, whatever its value, so after `ungetc` the position is one byte (or one `\r\n`) earlier. In binary mode this is the handle's position less the bytes buffered. In text mode, where the buffer holds translated bytes, `ftell` reads the bytes just before the handle's position again, untranslated (at most twice the number buffered), to find how many of them the buffered bytes came from, and puts the handle's position back; nothing else about the stream or handle changes. On a stream opened for writing it is the handle's position. Returns -1 with `errno`: from `lseek`, or `EINVAL` if the result would be before the start of the file (`ungetc` at offset 0). On a character device the result is whatever DOS reports and is not meaningful.
- **`int fseek(FILE *fp, long offset, int origin)`:** sets the file position to `offset` bytes from the start of the file (`SEEK_SET`), from the current position as `ftell` gives it (`SEEK_CUR`), or from the end (`SEEK_END`). Any byte offset is accepted in text mode as well as binary (C89 guarantees only `ftell` values there); positions past the end are allowed, as with `lseek`. The target is worked out before anything changes: if it would be before the start of the file (`EINVAL`), `origin` is not one of the three (`EINVAL`), or `ftell` or finding the end fails, `fseek` returns -1 and the stream and handle are unchanged. Otherwise the buffered input, any pushed-back byte and the end-of-file indicator are discarded, the handle is moved with `lseek` (which also drops a byte that text-mode `read` was holding), and 0 is returned; if that `lseek` fails, -1 with its `errno`. The error indicator is not changed.
- **`int feof(FILE *fp)`, `int ferror(FILE *fp)`:** the indicators; also macros.
- **`int fileno(FILE *fp)`:** the stream's DOS handle: 0, 1 and 2 for `stdin`, `stdout` and `stderr`. A macro only, as in Microsoft C 5.1's `<stdio.h>` (it is not C89), so it links nothing.
- **`int getc(FILE *fp)`, `int putc(int c, FILE *fp)`, `int putchar(int c)`:** `fgetc`, `fputc`, and `fputc` to `stdout`; also macros, which evaluate their arguments once.

Reading from a stream opened for writing, or writing to one opened for reading, fails with the error indicator set.

## Constraints

All are `__cdecl`. 8086 instructions only.

## Tests

`tests/stdio` (`seek`) covers `ftell` and `fseek` in binary and text mode: positions after reads that leave bytes buffered, across buffer refills, after `ungetc` (including `ungetc` of a different byte and at offset 0), on files with `\r\n`, a lone `\r`, `\r\r\n` and a `\r\n` that straddles a buffer refill; `fseek` back to every `ftell` value of a text file, reading the same bytes again; each origin, past the end, before the start (unchanged stream), a bad origin, clearing end of file but not the error indicator; and both on a stream opened for writing.
