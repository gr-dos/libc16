# Spec: stdio streams

**Header:** `<stdio.h>`  
**Members:** one per stream object (`stdin`, `stdout`, `stderr`) and one per function; private members `streams` (the stream table), `bufhook` (the pointers to the buffering engine), `bufio` (the engine) and `bufexit` (its termination routine, in assembly)  
**Tests:** `tests/stdio`, `tests/stdbuf`, `tests/bufperf`, `tests/process/bufkid`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- C89 7.9 (input/output), for every function listed here; for `fseek` and `ftell`, 7.9.9.2 and 7.9.9.4, with 7.9.7.11 (`ungetc`'s effect on the file position); for buffering, 7.9.3 (files), 7.9.5.2 (`fflush`), 7.9.5.5 (`setbuf`), 7.9.5.6 (`setvbuf`) and 7.10.4.3 (`exit` flushes the streams); C99 7.19.3 for when line-buffered output is written before input.
- The interface of Microsoft C 5.1's `fseek` and `ftell`: the Microsoft C 5.1 run-time library reference.
- `fdopen`: POSIX.1-2008 and the Microsoft C 5.1 run-time library reference; INT 21h function 4400h (IOCTL, get device information), to tell whether a handle is open: Ralf Brown's Interrupt List.
- `docs/spec/io.md`, which streams are built on; INT 21h function 40h (write), whose count says how many bytes DOS wrote: Ralf Brown's Interrupt List.
- gr-dos/libc16#34, which measured unbuffered output in `gr-dos/perl16` and set the requirements of "Buffering".

## Scope

libc16 implements the stream functions its consumers use (`docs/CONSUMERS.md`) with their C89 semantics, and no others: `fopen`, `fdopen`, `fclose`, `fflush`, `setvbuf`, `setbuf`, `fgetc`, `getc`, `fgets`, `fread`, `fputc`, `putc`, `putchar`, `fputs`, `fwrite`, `ungetc`, `fseek`, `ftell`, `feof`, `ferror`, `clearerr`, and Microsoft's `fileno`. The formatted functions (`printf` and the rest) are not provided (DESIGN.md §7; decision 3).

## Streams

- `stdin`, `stdout` and `stderr` are handles 0, 1 and 2, in the handle's mode (text unless changed with `setmode`). Each is its own member, so a program that uses only `stderr` links only `stderr`.
- **Output is unbuffered unless the program asks for a buffer** with `setvbuf` or `setbuf` ("Buffering"). An unbuffered output call reaches DOS (`write`) before it returns: nothing is lost when a program ends without closing or flushing, and output to `stdout` and `stderr` appears in the order it was written. Every stream starts unbuffered, the standard ones included, so a program that never calls `setvbuf` or `setbuf` behaves, and links, as before 0.16.
- **Input is buffered**, from `read`, so text-mode translation is `read`'s. `stdin`'s buffer is 128 bytes; a stream from `fopen` has its own.
- `fopen` and `fdopen` streams use one fixed table, so that neither needs the heap: with the standard streams, `FOPEN_MAX` (8) streams can be open at once. A further `fopen` or `fdopen` fails with `EMFILE`. The table and its buffers are a member of their own, so `fopen` does not link `fdopen` and `fdopen` does not link `fopen` or `open`.

## Functions

- **`FILE *fopen(const char *path, const char *mode)`:** `mode` is `r`, `w` or `a`, optionally followed by `b` (binary; otherwise text). `w` creates or truncates, `a` creates and appends (`O_APPEND`). On failure returns a null pointer with `errno` from `open`. The update modes (`+`) are not supported and fail with `EINVAL`, as does any other mode.
- **`FILE *fdopen(int fd, const char *mode)`:** a stream, in the table `fopen` uses, over the handle `fd`, which is already open (POSIX; Microsoft's `fdopen`). `mode` is `r`, `w` or `a`, optionally followed by `b` or `t`:
  - `r` makes a read stream and `w` and `a` a write stream. Neither changes the file: `w` does not truncate it, and nothing is created.
  - `a` also sets the handle's append mode, as `O_APPEND` does (`docs/spec/io.md`), so each write goes to the end of the file, as for `fopen`'s `a`.
  - `b` sets the handle to binary mode and `t` to text mode, as `setmode` does. With neither, the handle keeps its mode. (Microsoft's sets text mode then, from its default `_fmode`; libc16 keeps the mode, so that a stream over a handle from `dup` reads and writes as the original handle does.)
  
  The stream reads from, and writes at, the handle's position; its indicators are clear and it has nothing pushed back. `fileno` gives `fd`, and `fclose` closes `fd`. On failure `fdopen` returns a null pointer, leaving the handle open and its mode unchanged: `EINVAL` for any other `mode` (`+` included, as for `fopen`); `EBADF` if `fd` is not an open handle (checked with INT 21h 4400h), or is negative or 20 or more; `EMFILE` if the table is full. `fdopen` does not check that the handle was opened for reading or writing as `mode` says; a read or write that DOS refuses sets the stream's error indicator, as for any stream.
- **`int fclose(FILE *fp)`:** writes the stream's pending output, gives up its buffer (freeing one `setvbuf` allocated), and closes the stream and its handle; 0, or `EOF` if the write or `close` fails. The stream and its handle are closed either way, and output DOS did not take is then lost (C89 7.9.5.1 disassociates the stream whatever happens). The standard streams may be closed too.
- **`int fflush(FILE *fp)`:** writes `fp`'s pending output ("Buffering") and returns 0, or `EOF` with the error indicator set and `errno` from the write if DOS does not take all of it; what it did not take stays pending. `fflush(NULL)` does so for every stream with a buffer, and returns `EOF` if any write failed. An unbuffered stream has nothing pending: 0. On an input stream it does nothing and returns 0 (C leaves that undefined).
- **`int setvbuf(FILE *fp, char *buf, int mode, size_t size)`, `void setbuf(FILE *fp, char *buf)`:** "Buffering".
- **`int fgetc(FILE *fp)`:** the next byte as an `unsigned char` converted to `int`, or `EOF` with the end-of-file indicator set at end of file, or with the error indicator set on a read error.
- **`char *fgets(char *s, int n, FILE *fp)`:** reads at most `n - 1` bytes, stopping after a `\n`, and stores a terminating null. Returns `s`, or a null pointer if end of file came before any byte was read (leaving `s` unchanged) or a read error occurred.
- **`size_t fread(void *ptr, size_t size, size_t nmemb, FILE *fp)`:** reads up to `nmemb` elements of `size` bytes into `ptr`, as if by `fgetc` for each byte (a pushed-back byte first), and returns the number of complete elements read: fewer than `nmemb` only at end of file, with the end-of-file indicator set, or on a read error, with the error indicator set. A partly read element's bytes are stored but not counted. If `size` or `nmemb` is 0, returns 0 and reads nothing.
- **`size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *fp)`:** writes `nmemb` elements of `size` bytes from `ptr`, as if by `fputc` for each byte, and returns the number of complete elements written; fewer than `nmemb` only on a write error (including a full disk), with the error indicator set. On a buffered stream an element is written when its bytes are in the buffer or the file; after a failed write, the count is of the elements DOS wrote whole ("Errors"). If `size` or `nmemb` is 0, returns 0 and writes nothing.
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
- **A write stream's position** is its handle's, plus the bytes pending in its buffer, if it has one: the buffer holds the file's bytes ("Buffering"), so in text mode a pending `\n` counts 2.

- **`long ftell(FILE *fp)`:** the stream's position. With a byte pushed back at position 0 it returns -1L with `errno` `EINVAL`; if a DOS call it makes fails, -1L with `errno` as `docs/spec/io.md` maps the error. `ftell` changes nothing in the stream: its buffer, pushed-back byte, indicators, the handle's position and a byte kept by `read` are as they were when it returns, unless a DOS error leaves the position indeterminate ("Seek errors").
- **`int fseek(FILE *fp, long offset, int origin)`:** sets the position to `offset` bytes from the start (`SEEK_SET`), from the position `ftell` would return (`SEEK_CUR`), or from the end of the file (`SEEK_END`), and returns 0. Offsets count the file's bytes in text mode too, so any position `ftell` returned may be passed back with `SEEK_SET`, and `SEEK_CUR` and `SEEK_END` work as in binary mode. A position past the end of the file is allowed: reads there find the end of the file, and a write extends the file.
  - The target is worked out before the stream is changed. An `origin` other than the three, or a target before the start of the file, returns -1 with `errno` `EINVAL` and leaves the stream exactly as it was, pushed-back byte included. So does a failed `SEEK_CUR` from a pushed-back byte at position 0.
  - A stream with pending output writes it first, after checking `origin` (so an invalid `origin` writes nothing) and before working out the target, so that it goes where it was written. If that write fails, `fseek` returns -1 with `errno` from the write and the error indicator set, and moves nothing; the output DOS did not take stays pending ("Errors").
  - On success the buffer is emptied, a pushed-back byte and any byte kept by `read` are discarded, and the end-of-file indicator is cleared. The error indicator is not changed (C89 clears it only in `clearerr` and `rewind`).
  - If a DOS call fails, `fseek` returns -1 with `errno` as `docs/spec/io.md` maps the error, and the stream is as it was ("Seek errors").
  - A stream opened with `a` still writes at the end of the file (`O_APPEND`); `fseek` changes only where `ftell` and the next write start counting from.

### Seek errors

A DOS seek (INT 21h 42h) that fails does not move the handle, but an earlier one in the same call may have: `fseek` with `SEEK_END` moves the handle to the end to find its offset, and `ftell` in text mode may move it to look at the file. When a DOS call fails, `fseek` and `ftell` move the handle back to where it was before the call, if they had moved it, and then:

- **If the handle is where it was** (it never moved, or moving it back worked), the stream is exactly as before the call: its buffer, pushed-back byte, a byte kept by `read`, the indicators and its position. The call returns -1 (`fseek`) or -1L (`ftell`) with `errno` from the first DOS error.
- **If moving the handle back fails too,** the stream's position is indeterminate. The buffer, the pushed-back byte and a byte kept by `read` are discarded, so that the stream reads from wherever the handle is, the indicators are unchanged, and the call still returns -1 or -1L with `errno` from the first DOS error. A later successful `fseek` makes the stream's position definite again.

Neither function reports success after any DOS call in it has failed.

## Buffering

A write stream is **unbuffered**, **fully buffered** or **line-buffered** (C89 7.9.3). Every stream starts unbuffered; only `setvbuf` and `setbuf` change that, so libc16 has no buffering policy of its own and a program chooses one. A program that writes many small items to a file, as an interpreter does, should buffer it fully; one that writes to the console line-buffered, or leave it unbuffered:

    setvbuf(stdout, NULL, isatty(fileno(stdout)) ? _IOLBF : _IOFBF, BUFSIZ);

and, for each stream it opens for writing, `setvbuf(fp, NULL, _IOFBF, BUFSIZ)`. `stderr` is best left unbuffered, as C89 7.9.3 has it. The trade-offs: a buffer makes one DOS write of many output calls (`tests/bufperf`), but costs its memory, links the heap when `setvbuf` allocates it, and leaves output pending, so that it can be lost on an abnormal end and is overtaken by anything written to the handle directly ("Mixing streams and handles"). Line buffering on a device writes each line as it is finished, which is what a person at a console expects; full buffering on a device makes no output appear until the buffer fills.

### `setvbuf` and `setbuf`

- **`int setvbuf(FILE *fp, char *buf, int mode, size_t size)`** (C89 7.9.5.6): `mode` is `_IOFBF` (full), `_IOLBF` (line) or `_IONBF` (none), from `<stdio.h>`. Returns 0 on success, or a nonzero value with `errno` set and the stream as it was.
  - On a **write stream**, `setvbuf` first writes the stream's pending output, if it has a buffer already; if DOS does not take it all, `setvbuf` fails with `errno` from the write and changes nothing else. (C89 allows `setvbuf` only before any other operation on the stream; libc16 allows it at any time.) Then: `_IONBF` makes the stream unbuffered, ignoring `buf` and `size`. `_IOFBF` and `_IOLBF` give it `size` bytes at `buf` as its buffer, which must stay valid until the stream is closed or given another buffer; with a null `buf` it gets `size` bytes from `malloc` (`BUFSIZ`, 512, if `size` is 0), freed when the stream gives the buffer up. The buffer it had is given up (and freed, if `setvbuf` allocated it).
  - Fails with **`EINVAL`** for any other `mode`, or a buffer (`buf` not null, or a `size` other than 0 with a null `buf`) of fewer than 2 bytes, which cannot hold a text-mode `\r\n`; with **`ENOMEM`** if `malloc` fails, in which case the stream keeps the buffering it had (unbuffered, or its old buffer): a program that asks for a buffer it cannot have still writes correctly, only more slowly. Fails with `EBADF` for a closed stream, and with `EINVAL` for `_IOFBF` or `_IOLBF` once termination has begun ("Termination").
  - On a **read stream**, input is already buffered, by the stream's own buffer (128 bytes): `_IOFBF` returns 0 and changes nothing (`buf` is not used, as C89 allows), and `_IOLBF` and `_IONBF` cannot be honoured and fail with `EINVAL`.
- **`void setbuf(FILE *fp, char *buf)`** (C89 7.9.5.5): `setvbuf(fp, buf, _IOFBF, BUFSIZ)`, or with a null `buf`, `setvbuf(fp, NULL, _IONBF, 0)`; it returns nothing. It never allocates, so it links no heap.

### What a buffer holds, and when it is written

- The buffer holds the stream's **pending output as the bytes are to be in the file**: in text mode each `\n` is stored as `\r\n` when it is written to the stream, as `write` would translate it, using the handle's mode at that time. So a flush is one DOS write (INT 21h 40h) of the pending bytes, for an append stream after a seek to the end of the file, as `write` does for `O_APPEND`, and its count says exactly which bytes reached the file. A `\r\n` is never split between two flushes for want of room.
- Pending output is written:
  - when the next byte does not fit in the buffer (for a text-mode `\n`, when its 2 bytes do not);
  - on a line-buffered stream, at the end of each output call that wrote a `\n`: everything pending, including the bytes after the `\n`, in one write;
  - by `fflush` on the stream, or `fflush(NULL)`; by `fclose`; by `setvbuf` or `setbuf` on the stream;
  - by `fseek` on the stream (`ftell` writes nothing, and counts pending bytes in the position, "File positions");
  - for a **line-buffered** stream, whenever any stream reads from its handle to refill its buffer (`fgetc`, `getc`, `fgets`, and `fread` beyond the buffer), as C99 7.19.3 has it, so that a prompt appears before input is read; fully buffered streams are not written then. Input taken from a stream's buffer reads nothing and writes nothing;
  - by `system`, `libc16_systeme` and the spawn functions, for every stream, before the child runs (`docs/spec/process.md`), so that a child writing to an inherited handle writes after the parent's earlier output; if that write fails, the function runs no child and returns -1 with `errno` from the write;
  - at termination ("Termination").
- A binary-mode output call of at least the buffer's size is not copied: the pending output is written, then the call's bytes, each in one DOS write. Text-mode output always goes through the buffer, which is how it is translated.
- `fputc`, `fputs` and `fwrite` buffer the same way, so `putc`, `putchar` and the formatted functions of `gr-dos/libkitchen16`, which write through `fwrite`, are buffered too.

### Errors

- A flush that DOS does not complete sets the stream's error indicator and `errno`: as `docs/spec/io.md` maps the DOS error, or `ENOSPC` when DOS writes fewer bytes than asked (a full disk). The bytes it wrote are in the file; the rest of the pending output stays pending, at the start of the buffer, to be written by a later flush.
- An output call that flushes and fails, fails: `fputc` returns `EOF`, `fputs` `EOF`, and `fwrite` the number of its elements that DOS wrote whole (a text-mode `\n` is written when both bytes of its `\r\n` are). None of the failing call's own bytes is left pending; bytes of earlier calls, which succeeded, are. So output is never dropped without a failure being reported for it: either the call that wrote it failed, or it stays pending, and `ferror` and `fflush`'s result report that it has not reached the file. A program can `clearerr` and `fflush` again, for example once there is room on the disk.
- `fclose` reports a failed flush with `EOF`; the output DOS did not take is then lost. Termination ignores a failed flush; the exit status is not changed.
- `ferror`, `clearerr` and `feof` work on buffered streams as on unbuffered ones; `clearerr` does not discard pending output.

### Termination

- **`exit` and returning from `main`** write every stream's pending output (C89 7.10.4.3), through a routine in the termination table (`docs/spec/startup.md`) that is linked with `setvbuf` and `setbuf`. The routine then makes every stream unbuffered, and from then on `setvbuf` and `setbuf` give no stream a buffer. So output written after it, by an `atexit` function or another termination routine, goes straight to DOS, after what was pending: the output is in the order it was written whichever of the termination table's routines runs first. Streams are not closed; DOS closes the handles.
- **Not written:** pending output is lost when the program ends any other way: `abort` (`docs/spec/stdlib.md`), the default action of a signal (`docs/spec/signal.md`), Ctrl-C ending the program through DOS, INT 21h function 4Ch or 00h called directly, and a program with its own entry point (`docs/spec/startup.md`) that does not end through `exit`. libc16 has no `_exit`. A program that must not lose output in these cases flushes first.

### Mixing streams and handles

A stream's pending output is in the program's memory, not yet in the file, and nothing outside the stream functions knows of it. The program must flush a buffered stream before:

- writing to its handle (`fileno(fp)`) with `write`, or to another handle for the same file or device, such as one from `dup`; otherwise those bytes overtake the pending output (`tests/stdbuf/bufline`);
- `dup2` onto its handle, closing its handle with `close`, `lseek` or `chsize` on it, or `setmode` (pending bytes were translated with the old mode, and stay so);
- handing the handle to a child some other way than `system`, `libc16_systeme` and the spawn functions, which flush.

Output to `stdout` and `stderr` is in order only while both are unbuffered, or after a flush. Reading a file through another stream or handle sees only the output that has been flushed; `filelength` on the stream's own handle gives the length without the pending bytes.

### Memory

- A buffer is the caller's (`setbuf`, or `setvbuf` with a `buf`), or from `malloc`: in the small model from `DGROUP`, and in the compact and large models from the far heap, outside `DGROUP`. No stream has a buffer of its own in static storage. (The stream table's input buffers, 128 bytes for each of its 5 streams, are unchanged.)
- `DGROUP` is otherwise unchanged in a program that does not call `setvbuf` or `setbuf`, but for 2 code pointers (4 bytes, 8 in the large model) of BSS that the output functions share. One that does adds a list of the buffered streams (`FOPEN_MAX` data pointers), one code pointer, one byte and a termination-table entry: 21 bytes in the small model, 37 in the compact and 41 in the large.

### Performance

`tests/bufperf/perf` measures each model under DOSBox-X (an 8086, `cputype=8086`, at DOSBox-X's default speed). An interpreter's print of four items a line, 2,000 lines to a file in text mode, makes 8,000 DOS writes unbuffered and 59 with a 512-byte buffer; 20,000 single bytes make 20,000 and 40. 64 text-mode blocks of 4,096 bytes make 2,112 writes unbuffered (`write` translates through 128 bytes of stack) and 519 buffered, and take no longer. The test fails if a buffered run makes more than a twentieth of the writes or takes as long, or if the blocks are slower buffered. `tests/stdbuf/wcount` checks the counts exactly: one write per buffer for full buffering, at most `BUFSIZ` - 1 bytes short of full; one per line for line buffering; one for each large binary call.

### Linking

- `setvbuf` links the buffering engine and `malloc` and `free`; `setbuf` the engine alone. The engine (`bufio`, with the termination routine `bufexit`) links the DOS write and `memcpy`, and not the stream table, `fopen`, `fdopen` or the standard streams.
- The stream functions reach the engine through two pointers, in a member of their own (`bufhook`), which `setvbuf` and `setbuf` set. `fputc`, `fputs`, `fwrite`, `fflush`, `fclose`, `fseek`, `fgetc` and `fread`, and `system`, `libc16_systeme` and the spawn functions, link `bufhook` and nothing else on account of buffering: a program that never calls `setvbuf` or `setbuf` links no buffering code, no heap and no termination routine, and its streams are unbuffered as before. `ftell` and the other stream members link nothing new.
- `FILE` keeps its size and layout, and the bits of `_flag` that `feof`, `ferror` and `clearerr` test; three bits that were unused mark a buffered stream (`<stdio.h>`). A program or library compiled with the headers of an earlier release works with this one, and `gr-dos/libkitchen16`'s formatted output needs no change.

## Constraints

All are `__cdecl`. 8086 instructions only. Moving the stream table out of `fopen` for `fdopen` may change `fopen`'s size, but not what a program that calls it links besides the table. `fseek` and `ftell` cost nothing in a program that calls neither: the other members, `fgetc` and `fopen` among them, and `FILE` do not change for them, so such a program links the same members as before, of the same size. Buffering costs a program that does not ask for it only `bufhook` and the few bytes each stream function needs to test for a buffer ("Buffering", "Linking").

## Tests

`tests/stdbuf` covers buffering in every model, with `tests/stdbuf/wrhook.asm`, a hook on INT 21h that counts the DOS writes to a handle and can make them write fewer bytes than asked, or fail with a DOS error:

- `setvbuf`: invalid modes and sizes; full buffering in the caller's buffer and from `malloc` (`BUFSIZ` for a size of 0); text mode's `\r\n` in the buffer, unsplit; line buffering; changing the buffer and `_IONBF` writing what is pending; `setbuf`; read streams; line-buffered output written before input that reads, and not before input from the buffer; `fflush(NULL)`; `fseek` and `ftell` with pending output, an invalid `origin` writing nothing; append streams; `fdopen` and a handle's own write after `fflush`; `malloc` failing; a closed stream.
- `wfail`: short writes and a full disk, DOS errors, and what each leaves pending, written and counted, for `fputc`, `fputs`, `fwrite` (whole elements, and a text-mode `\n` half written), `fflush`, `fflush(NULL)`, `fseek`, `setvbuf`, `fclose` (which still closes the handle), line buffering, and large binary calls.
- `wcount`: the number of DOS writes for 2,000 lines of four items through `fputs`, `fwrite` and `putc`, fully and line-buffered, with 512 and 4,096 bytes, in text and binary mode, each file the same as unbuffered output makes it; large binary calls; text through the buffer.
- `bufout`, `bufexit`, `bufabrt` and `bufchk`: termination writes pending output, for standard output and a file, returning from `main` and through `exit`, with an `atexit` function writing and calling `setvbuf` after; `abort` loses it.
- `bufline`: line-buffered standard output, and `write` to its handle overtaking a partial line.
- `setbuf`: standard output in the program's own buffer, linking no heap (`tests/stdbuf/members`).

`tests/process/bufkid` checks that `spawnl`, `system` and `libc16_systeme` write the pending output, a partial line included, before the child's, and `tests/process/buffail` that a failed write stops the child. `tests/bufperf/perf` measures the time and writes ("Performance"). The members files of every suite check that a program that does not call `setvbuf` or `setbuf` links no more than `bufhook` for it.

`tests/stdio/fdopen` covers `fdopen` with `r`, `w`, `a`, `rb`, `wt` and `ab` over handles from `open` and from `dup`, reading and writing through the stream and the handle's position before and after; `b`, `t` and neither, each against `setmode`'s report of the handle's mode; `w` leaving the file's length alone; `a` appending after a seek to the start; `fileno`; `fclose` closing the handle; invalid modes (`r+`, `x`, the empty string); a closed handle, -1 and 20; and a full table, with five streams already open, leaving the handle usable. It also writes a line to standard output through `fdopen(dup(1), "w")`, which must appear in its output.
