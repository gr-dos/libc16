# Spec: low-level I/O and errno

**Headers:** `<io.h>`, `<fcntl.h>`, `<sys/stat.h>`, `<errno.h>`  
**Members:** `errno`; one per function; private members for the handle table and DOS error mapping  
**Tests:** `tests/io`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- INT 21h functions 3Ch (create), 3Dh (open), 3Eh (close), 3Fh (read), 40h (write; a count of 0 truncates at the file pointer), 42h (move file pointer), 43h (get attributes) and their error codes: Ralf Brown's Interrupt List; MS-DOS 2.0 `SYSCALL.DOC`. All are DOS 2.0 functions.
- The interfaces, flag values and text-mode behaviour of Microsoft C 5.1's `open`, `read`, `write`, `lseek`, `close`, `filelength`, `chsize`, `access` and `setmode`: the Microsoft C 5.1 run-time library reference.
- POSIX.1-2008 for details Microsoft's reference leaves open (`O_EXCL`, `lseek` beyond the start).

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

## Constraints

All are `__cdecl`. 8086 instructions only. A program that uses only binary handles still links the text-mode code of `read` and `write`, which is small.
