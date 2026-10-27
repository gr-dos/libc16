# Spec: file status, permissions and directories

**Headers:** `<sys/stat.h>`, `<sys/types.h>`, `<sys/utime.h>`, `<io.h>`, `<direct.h>`  
**Members:** `stat`, `chmod`, `mkdir`, `utime`, and the private `dostime` (DOS dates and times to and from `time_t`)  
**Tests:** `tests/files` (run by `tests/run-dos`, in every model)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- The interfaces of Microsoft C 5.1's `stat`, `chmod`, `mkdir` and `utime`, `struct stat`, `struct utimbuf` and the `S_IF*` and `S_I*` values: the Microsoft C 5.1 run-time library reference.
- INT 21h functions 19h (current drive), 2Ah and 2Ch (date and time), 39h (make directory), 3Dh and 3Eh (open, close), 4300h and 4301h (attributes), 4Eh (find first), 5701h (set a file's date and time), and DOS's packed date and time: Ralf Brown's Interrupt List.
- `docs/spec/time.md` for `time_t` and the time zone; `docs/spec/io.md` for `errno`.

## DOS dates and times

DOS stores a file's modification time as local time to two seconds: the date in a word (bits 15–9 the year from 1980, 8–5 the month, 4–0 the day) and the time in another (bits 15–11 the hour, 10–5 the minute, 4–0 the seconds divided by 2). Converting one to a `time_t` takes `TZ` into account as `localtime` does (`tzset` is called first if it has not been): the time is taken as daylight saving time if the zone has one and that time falls inside it, and the hour that repeats when daylight saving time ends is taken as daylight time. A month of 0 is taken as January and a day of 0 as the 1st. Converting a `time_t` to a DOS date and time uses `localtime` and rounds the seconds down to an even number; years before 1980 or after 2107 cannot be stored.

## `int stat(const char *path, struct stat *buf)`

Fills `*buf` for the file or directory `path` and returns 0, or returns -1 with `errno` set to `ENOENT` if there is none (including when `path` contains `*` or `?`).

- `st_mode`: `S_IFDIR` for a directory, `S_IFREG` otherwise; `S_IREAD` always; `S_IWRITE` unless the read-only attribute is set; `S_IEXEC` for a directory and for a file whose name ends in `.EXE`, `.COM` or `.BAT`. These owner bits (01C0h) are copied into the group (0038h) and other (0007h) bits, so a writable file is octal 100666, a read-only one 100444 and a directory 040777 (Microsoft's later libraries do the same).
- `st_size`: the size in bytes; 0 for a directory.
- `st_mtime`, `st_atime`, `st_ctime`: the modification time (DOS records no other), as a `time_t`.
- `st_dev`, `st_rdev`: the drive, 0 for A:, from the path or else the current drive. `st_ino` 0, `st_nlink` 1, `st_uid` and `st_gid` 0.
- A root directory (`C:\`, `\`, `C:`, and paths that resolve to one with `.` and `..`) has no directory entry; it is reported as a directory, writable, size 0, with the time 1980-01-01 00:00:00 local time.

Hidden and system files and directories are found too.

## `int chmod(const char *path, int pmode)`

Sets the read-only attribute of `path` if `pmode` does not include `S_IWRITE`, and clears it if it does; other attributes are unchanged and `S_IREAD` is ignored (DOS files are always readable). Returns 0, or -1 with `errno` (`ENOENT` if `path` does not exist).

## `int mkdir(const char *path)`

Creates the directory `path` (INT 21h 39h). Returns 0, or -1 with `errno`: `EACCES` if it exists or cannot be created, `ENOENT` if the path to it does not exist.

## `int utime(const char *path, struct utimbuf *times)`

Sets the modification time of the file `path` to `times->modtime` (`times->actime` is ignored; DOS records no access time), or to the current date and time if `times` is a null pointer. The file is opened for reading (so read-only files can be changed too), its date and time set with INT 21h 5701h, and closed. Returns 0, or -1 with `errno`: from opening the file (`ENOENT`; `EACCES` for a directory, which DOS cannot open), or `EINVAL` if `times->modtime` is before 1980 or after 2107 in local time.

## Constraints

- All are `__cdecl` (`docs/ABI.md`). `stat` and `utime` link the time zone members (`docs/spec/time.md`), and so `getenv`; `chmod` and `mkdir` do not.
- 8086 instructions only.

## Tests

`tests/files` covers: `stat` of a file, a read-only file, a hidden file, a program, a directory, `.` and `..`, the root in each spelling, a missing file and a wildcard; `st_mtime` against the time `utime` set, with `TZ` unset and with `EST5EDT` across both daylight saving changes; `utime` with a null pointer (the time DOS's clock gives), on a read-only file, a directory, a missing file and out-of-range times; `chmod` both ways; `mkdir` of a new, an existing and an unreachable directory. DOSBox-X keeps the attributes of a file that has had any in a second host file and from then on reports that file's size and time for it, so the tests check sizes and times only on files whose attributes they never change.
