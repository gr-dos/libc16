# Spec: stdio streams

**Header:** `<stdio.h>`  
**Members:** one per stream object (`stdin`, `stdout`, `stderr`) and one per function  
**Tests:** `tests/stdio`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- C89 7.9 (input/output), for every function listed here; for `fseek` and `ftell`, 7.9.9.2 and 7.9.9.4, with 7.9.7.11 (`ungetc`'s effect on the file position).
- The interface of Microsoft C 5.1's `fseek` and `ftell`: the Microsoft C 5.1 run-time library reference.
- `docs/spec/io.md`, which streams are built on.

## Scope

libc16 implements the stream functions its consumers use (`docs/CONSUMERS.md`) with their C89 semantics, and no others: `fopen`, `fclose`, `fflush`, `fgetc`, `getc`, `fgets`, `fread`, `fputc`, `putc`, `putchar`, `fputs`, `fwrite`, `ungetc`, `fseek`, `ftell`, `feof`, `ferror`, `clearerr`, and Microsoft's `fileno`. The formatted functions (`printf` and the rest) are not provided (DESIGN.md §7; decision 3).

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
- **`int feof(FILE *fp)`, `int ferror(FILE *fp)`:** the indicators; also macros.
- **`void clearerr(FILE *fp)`:** clears the end-of-file and error indicators (C89 7.9.10.1), and nothing else: the buffer, a pushed-back byte and the position are unchanged. Also a macro, which evaluates `fp` once.
- **`int fileno(FILE *fp)`:** the stream's DOS handle: 0, 1 and 2 for `stdin`, `stdout` and `stderr`. A macro only, as in Microsoft C 5.1's `<stdio.h>` (it is not C89), so it links nothing.
- **`int getc(FILE *fp)`, `int putc(int c, FILE *fp)`, `int putchar(int c)`:** `fgetc`, `fputc`, and `fputc` to `stdout`; also macros, which evaluate their arguments once.

Reading from a stream opened for writing, or writing to one opened for reading, fails with the error indicator set.

## File positions

A stream's **position** is the offset in the file, in bytes, of the next byte a read would take from the file, or a write would write. Bytes the stream has read into its buffer and not yet returned are not yet read: the position is before them.

- **Binary mode:** the buffer's bytes are the file's bytes, so the position is the handle's position less the bytes left in the buffer.
- **Text mode:** the position counts the file's bytes, not the translated ones. A `\n` that `read` made from `\r\n` stands for two bytes of the file, and the position before it is that of the `\r`; every other byte stands for one. A byte that `read` keeps after a `\r` (`docs/spec/io.md`) is in the file before the handle's position, and is not yet read. At the `^Z` that ends a text file the position is the `^Z`'s offset.
- **A byte pushed back by `ungetc`** moves the position back by one byte, in either mode, until it is read again or discarded. (In text mode C89 leaves the position unspecified here; one byte back is where a `\n` from `\r\n`, or any other byte, would be read again.) At position 0 there is no position before it.
- **Output is unbuffered,** so a write stream's position is its handle's.

- **`long ftell(FILE *fp)`:** the stream's position. With a byte pushed back at position 0 it returns -1L with `errno` `EINVAL`; if a DOS call it makes fails, -1L with `errno` as `docs/spec/io.md` maps the error. `ftell` changes nothing in the stream: its buffer, pushed-back byte, indicators, the handle's position and a byte kept by `read` are as they were when it returns, unless a DOS error leaves the position indeterminate ("Seek errors").
- **`int fseek(FILE *fp, long offset, int origin)`:** sets the position to `offset` bytes from the start (`SEEK_SET`), from the position `ftell` would return (`SEEK_CUR`), or from the end of the file (`SEEK_END`), and returns 0. Offsets count the file's bytes in text mode too, so any position `ftell` returned may be passed back with `SEEK_SET`, and `SEEK_CUR` and `SEEK_END` work as in binary mode. A position past the end of the file is allowed: reads there find the end of the file, and a write extends the file.
  - The target is worked out before the stream is changed. An `origin` other than the three, or a target before the start of the file, returns -1 with `errno` `EINVAL` and leaves the stream exactly as it was, pushed-back byte included. So does a failed `SEEK_CUR` from a pushed-back byte at position 0.
  - On success the buffer is emptied, a pushed-back byte and any byte kept by `read` are discarded, and the end-of-file indicator is cleared. The error indicator is not changed (C89 clears it only in `clearerr` and `rewind`).
  - If a DOS call fails, `fseek` returns -1 with `errno` as `docs/spec/io.md` maps the error, and the stream is as it was ("Seek errors").
  - A stream opened with `a` still writes at the end of the file (`O_APPEND`); `fseek` changes only where `ftell` and the next write start counting from.

### Seek errors

A DOS seek (INT 21h 42h) that fails does not move the handle, but an earlier one in the same call may have: `fseek` with `SEEK_END` moves the handle to the end to find its offset, and `ftell` in text mode may move it to look at the file. When a DOS call fails, `fseek` and `ftell` move the handle back to where it was before the call, if they had moved it, and then:

- **If the handle is where it was** (it never moved, or moving it back worked), the stream is exactly as before the call: its buffer, pushed-back byte, a byte kept by `read`, the indicators and its position. The call returns -1 (`fseek`) or -1L (`ftell`) with `errno` from the first DOS error.
- **If moving the handle back fails too,** the stream's position is indeterminate. The buffer, the pushed-back byte and a byte kept by `read` are discarded, so that the stream reads from wherever the handle is, the indicators are unchanged, and the call still returns -1 or -1L with `errno` from the first DOS error. A later successful `fseek` makes the stream's position definite again.

Neither function reports success after any DOS call in it has failed.

## Constraints

All are `__cdecl`. 8086 instructions only. `fseek` and `ftell` cost nothing in a program that calls neither: the other members, `fgetc` and `fopen` among them, and `FILE` do not change for them, so such a program links the same members as before, of the same size.
