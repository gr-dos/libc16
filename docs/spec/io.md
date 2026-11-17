# Spec: low-level I/O and errno

**Headers:** `<io.h>`, `<fcntl.h>`, `<sys/stat.h>`, `<errno.h>`  
**Members:** `errno`; one per function (`isatty`, `dup` and `dup2` included); private members for the handle table and DOS error mapping  
**Tests:** `tests/io`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- INT 21h functions 3Ch (create), 3Dh (open), 3Eh (close), 3Fh (read), 40h (write; a count of 0 truncates at the file pointer), 42h (move file pointer), 43h (get attributes) and their error codes: Ralf Brown's Interrupt List; MS-DOS 2.0 `SYSCALL.DOC`. All are DOS 2.0 functions.
- The interfaces, flag values and text-mode behaviour of Microsoft C 5.1's `open`, `read`, `write`, `lseek`, `close`, `filelength`, `chsize`, `access` and `setmode`: the Microsoft C 5.1 run-time library reference.
- POSIX.1-2008 for details Microsoft's reference leaves open (`O_EXCL`, `lseek` beyond the start, `dup2` of a handle to itself).
- `isatty`, `dup`, `dup2`: the Microsoft C 5.1 run-time library reference; INT 21h functions 4400h (IOCTL, get device information: bit 7 of `DX` is set for a device), 45h (duplicate handle) and 46h (force duplicate handle, which closes the second handle first if it is open): Ralf Brown's Interrupt List. Under DOSBox-X, 46h with both handles the same fails with error 6 and leaves the handle open, and 46h to handle 25 succeeds, writing past the program's 20-entry handle table (our own experiment).

## `errno`

`int errno` (symbol `_errno`), zero at startup. Functions set it only on failure. A DOS error code maps to `errno` as follows:

| DOS error | `errno` |
|---|---|
| 1 invalid function, 9 invalid memory block, 12 invalid access code, 13 invalid data | `EINVAL` |
| 2 file not found, 3 path not found, 15 invalid drive, 18 no more files | `ENOENT` |
| 4 too many open files | `EMFILE` |
| 5 access denied, 16 removing the current directory, and any code not listed | `EACCES` |
| 6 invalid handle | `EBADF` |
| 7 memory control blocks destroyed, 8 insufficient memory | `ENOMEM` |
| 10 invalid environment | `E2BIG` |
| 11 invalid format | `ENOEXEC` |
| 17 not the same device | `EXDEV` |
| 80 file exists | `EEXIST` |

## Handles and modes

A handle (file descriptor) is a DOS handle. libc16 keeps a mode for handles 0 to 19 (the DOS default limit): **text** (the default, decision 9) or **binary**, and whether the handle **appends**. Handles 0 to 4, inherited from DOS, start in text mode. A handle outside 0 to 19, negative ones included, has no mode: the functions pass it to DOS, which fails with `EBADF` unless it is open, and never look it up in the table.

**Text mode** applies to `read` and `write` only:

- `write` writes each `\n` as `\r\n`.
- `read` returns each `\r\n` as `\n`, a lone `\r` unchanged, and stops at a `^Z` (1Ah), which is the end of the file: that read returns the bytes before it and the file pointer is left at the `^Z`, so later reads return 0. A `\r\n` split across two reads is still returned as `\n`; when a `\r` ends what DOS returned, `read` reads one more byte to decide, and a byte that is not `\n` is kept for the next `read` on the handle.

## Functions

- **`int open(const char *path, int oflag, ...)`:** opens `path` and returns its handle, or -1. `oflag` is one of `O_RDONLY`, `O_WRONLY`, `O_RDWR`, combined with any of:
  - `O_CREAT`: if the file does not exist, create it, with the read-only attribute unless the third argument (`int pmode`, read only with `O_CREAT`) includes `S_IWRITE`;
  - `O_EXCL` with `O_CREAT`: fail with `EEXIST` if the file exists;
  - `O_TRUNC`: truncate an existing file opened for writing to 0 bytes;
  - `O_APPEND`: move to the end of the file before every `write`;
  - `O_TEXT` or `O_BINARY` (both: `EINVAL`).
  
  An access mode of 3 is `EINVAL`. A handle of 20 or more is closed and `open` fails with `EMFILE`.
- **`int close(int fd)`:** closes the handle; 0, or -1.
- **`int read(int fd, void *buf, unsigned n)`:** reads up to `n` bytes, translated in text mode; returns the number stored, 0 at end of file or when `n` is 0, or -1.
- **`int write(int fd, const void *buf, unsigned n)`:** writes `n` bytes, translated in text mode, and returns `n`; or -1. Writing fewer bytes than asked (a full disk) is -1 with `ENOSPC`. `n` of 0 writes nothing and returns 0 (it never truncates).
- **`long lseek(int fd, long offset, int origin)`:** moves the file pointer to `offset` from the start (`SEEK_SET`), the current position (`SEEK_CUR`) or the end (`SEEK_END`) and returns the new position, or -1L. An invalid `origin`, or a resulting position before the start, is `EINVAL`, and the position is unchanged. A byte kept by a text-mode `read` is discarded, and `SEEK_CUR` counts from just before it.
- **`long filelength(int fd)`:** the length of the file in bytes, or -1L; the position is unchanged.
- **`int chsize(int fd, long size)`:** extends the file with zero bytes or truncates it to `size` bytes; the position is unchanged. 0, or -1: `EINVAL` for a negative `size`, `ENOSPC` if the disk fills, `EACCES` if the handle was not opened for writing. If a seek to find the end of the file or the new end fails, the size is unchanged and the seek's error is returned. If only restoring the position fails, the size has changed but the result is still -1 with that error; a restore that fails after an earlier error leaves `errno` as that error set it.
- **`int access(const char *path, int mode)`:** 0 if `path` (a file or directory) exists and allows `mode`: 0 existence, 2 write, 4 read, 6 both. A read-only file fails 2 and 6 with `EACCES`; a missing one fails with `ENOENT`; any other `mode` is `EINVAL`. Returns 0 or -1.
- **`int setmode(int fd, int mode)`:** sets the handle's mode to `O_TEXT` or `O_BINARY` and returns the previous one; -1 with `EINVAL` for any other `mode`, or `EBADF` for a handle outside 0 to 19.

## Handles and devices

- **`int isatty(int fd)`:** 1 if `fd` is an open handle to a character device (bit 7 of the device information from INT 21h 4400h), 0 if it is an open handle to a file. The console is a device, and so are `NUL`, `PRN` and the serial ports: DOS cannot tell a terminal from another device, and neither can `isatty`, as in Microsoft C. If `fd` is not an open handle (negative ones included), it returns 0 with `errno` `EBADF`. It changes nothing about the handle.
- **`int dup(int fd)`:** returns a new handle, the lowest free one, for what `fd` refers to (INT 21h 45h), or -1. The two handles share the file position, as DOS handles do, and closing one leaves the other open. The new handle gets `fd`'s mode: text or binary, and whether it appends. A byte that a text-mode `read` kept for `fd` stays with `fd`; the new handle has none. Errors: `EBADF` if `fd` is not an open handle; `EMFILE` if the program has no free handle. A new handle of 20 or more, which has no mode, is closed and `dup` fails with `EMFILE`, as `open` does.
- **`int dup2(int fd, int fd2)`:** makes `fd2` refer to what `fd` refers to (INT 21h 46h) and returns 0, as Microsoft C's does (POSIX's returns `fd2`; a caller that tests for -1 works with either). If `fd2` was open, it is closed first, as by DOS: a stream that used `fd2` now reads and writes `fd`'s file through it, which is how a program redirects its standard output. `fd2` gets `fd`'s mode, as for `dup`, and any byte kept for `fd2` is discarded. Errors, with `fd2` left as it was:
  - `EBADF` if `fd` is not an open handle;
  - `EBADF` if `fd2` is negative or 20 or more, without calling DOS: such a handle could have no mode, and DOS may not check it against the size of the program's handle table.
  
  If `fd2` equals `fd` and `fd` is open, `dup2` returns 0 and changes nothing, without INT 21h 46h, which may fail or close the handle in that case.

## Constraints

All are `__cdecl`. 8086 instructions only. A program that uses only binary handles still links the text-mode code of `read` and `write`, which is small. `isatty`, `dup` and `dup2` are each their own member, and none links `open`, `read` or `write`.

## Tests

`tests/io/io` covers the functions above `isatty`. `tests/io/isatty` covers `isatty` on handle 0 (the console under the test runner), 1 (redirected to a file), 2 (the console), `CON` and `NUL` opened by name, a file, a closed handle, -1 and 20. `tests/io/dup` covers `dup` of a binary file handle, an appending handle and a handle with a kept byte (it stays with the old handle), the shared position, closing either handle, a closed handle, and running out of handles; `dup2` onto an open handle (whose file is closed and whose mode and kept byte are replaced), onto a closed one, onto itself, from a closed handle, and to -1 and 20; and redirecting handle 1 to a file and back with `dup` and `dup2`.
