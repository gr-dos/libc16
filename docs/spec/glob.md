# Spec: wildcard expansion of file operands

**Header:** `<libc16.h>`  
**Members:** `glob` (`libc16_glob`), `globfree` (`libc16_globfree`)  
**Tests:** `tests/glob` (`glob`, and the example consumer `gcat`)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

DOS's `COMMAND.COM` passes `*.TXT` to a program as it is typed; a program that takes file operands expands them itself (#41, for `gr-dos/cat16`). `libc16_glob` expands one operand with DOS's own directory search, INT 21h 4Eh and 4Fh (`_dos_findfirst` and `_dos_findnext`, `docs/spec/dos.md`), and collects the paths. It is opt-in: the startup never expands `argv`, and a program calls it only for the operands that name files, since others (a search string, say) may contain `*` or `?` meant literally. It is libc16's own interface, so its names have the `libc16_` prefix (decision 3); the shape is POSIX's `glob`, with DOS's patterns and no Unix extensions.

## Sources

- INT 21h 4Eh and 4Fh, DOS 2.0, and the attribute byte of a directory entry: Ralf Brown's Interrupt List; MS-DOS 2.0 `SYSCALL.DOC`.
- POSIX.1-2001 `glob` and `globfree`, for the shape of the interface: the structure, `GLOB_APPEND`, `GLOB_NOCHECK`, `GLOB_NOSORT`, `GLOB_NOMATCH` and `GLOB_NOSPACE`.
- The issue (#41) and the two consumers it names, `gr-dos/cat16#1` and `gr-dos/ls16#1`.
- libc16's own `docs/spec/dos.md`, `heap.md`, `string.md` and `stdlib.md`, and DOSBox-X experiments (below).

## Interface

```c
typedef struct {
    unsigned gl_pathc;      /* paths in gl_pathv */
    char **gl_pathv;        /* gl_pathc paths, then a null pointer */
    unsigned __gl_slots;    /* private */
} libc16_glob_t;

int  libc16_glob(const char *pattern, unsigned flags, libc16_glob_t *g);
void libc16_globfree(libc16_glob_t *g);
```

Both are `__cdecl`. `flags` is 0 or an OR of:

| Flag | Meaning |
|---|---|
| `LIBC16_GLOB_APPEND` | add to the paths already in `*g`, which holds the result of earlier calls or is all zeros |
| `LIBC16_GLOB_NOCHECK` | a wildcard pattern that matches nothing gives the pattern itself, as one path |
| `LIBC16_GLOB_NOSORT` | leave the matches in the order DOS returns them |
| `LIBC16_GLOB_DIRS` | directories match as well as files |
| `LIBC16_GLOB_HIDDEN` | entries with the Hidden or System attribute match as well |

The result is 0 or one of:

| Result | When | `errno` |
|---|---|---|
| `LIBC16_GLOB_NOMATCH` (1) | a wildcard pattern matched nothing, without `LIBC16_GLOB_NOCHECK` | `ENOENT` |
| `LIBC16_GLOB_NOSPACE` (2) | `malloc` or `realloc` failed, or the paths would need more than 64 KiB of pointers | `ENOMEM` |
| `LIBC16_GLOB_ERROR` (3) | a wildcard before the last component (no DOS call is made), or the search failed other than by finding nothing: a missing directory, an invalid drive | `EINVAL` for the wildcard; otherwise the search's, with `_doserrno` its DOS code (`docs/spec/io.md`) |

## Patterns

A pattern is a DOS path: an optional drive (`C:`), then directories separated by `\` or `/`, then a last component. `*` and `?` may appear only in the last component, the part after the last `\`, `/` or `:`; elsewhere the result is `LIBC16_GLOB_ERROR` with `EINVAL`.

- **No wildcard.** A pattern with no `*` or `?` is added as it is, and 0 returned, without searching: whether it exists, or is a file, a directory or a device (`CON`, `NUL`), is for the program to find when it opens it. So a program can pass every file operand through `libc16_glob` and get `cat`'s usual "no such file" from `open` for a literal name that does not exist. `LIBC16_GLOB_DIRS`, `LIBC16_GLOB_HIDDEN` and `LIBC16_GLOB_NOCHECK` do not apply.
- **Matching is DOS's.** The pattern is passed to INT 21h 4Eh as it is, so `*` and `?` mean what they mean to DOS in an 8.3 name, case-insensitively: `*` fills the rest of the name or the extension with `?`, so what follows it there is ignored (`F*1.*` is `F*.*`), and `?` matches one character or the padding after a short name. So `FILE?.DAT` matches `FILE1.DAT` and `FILE.DAT` but not `FILE10.DAT`, and a `*` with no dot matches only names without an extension, as in DOS: `*.*` matches every name (*tested*).
- **Drives and directories.** The directory and drive are DOS's too: a relative pattern (`DOCS\*.TXT`, `..\*.C`) is relative to the current directory of its drive; `C:*.TXT` is relative to drive C:'s current directory, which need not be the current drive's, and `C:\DOCS\*.TXT` is absolute (*tested*, across two drives).
- **The paths.** Each path is the pattern up to and including its last `\`, `/` or `:`, exactly as given, followed by the name DOS returns (uppercase on a FAT disk): `C:*.TXT` gives `C:A.TXT`, and `docs/*.txt` gives `docs/A.TXT`. A path is therefore usable from the current directory just as the pattern was.

## What matches

The search asks DOS for ordinary files, with directories added by `LIBC16_GLOB_DIRS` and Hidden and System entries by `LIBC16_GLOB_HIDDEN` (attribute `_A_SUBDIR`, `_A_HIDDEN | _A_SYSTEM`), and then checks every entry it returns itself, since DOSBox-X, for one, returns Hidden and System files whatever the attribute asked for (*tested*). An entry matches when:

- it is not a volume label (`_A_VOLID` set), which is never a path: that excludes VFAT long-name slots too (attribute 0Fh), whatever the flags (*tested* for the label);
- it is not `.` or `..`, which a pattern such as `*.*` finds in a subdirectory (and DOSBox-X, in a mounted host directory's root);
- it is not a directory, unless `LIBC16_GLOB_DIRS`;
- it has neither Hidden nor System, unless `LIBC16_GLOB_HIDDEN`.

Read-only and Archive do not matter (*tested*).

## Order, duplicates and appending

- **Sorted.** Unless `LIBC16_GLOB_NOSORT`, the paths one call adds are sorted, case-insensitively, as `stricmp` orders them (letters compared as lowercase, so `_` comes before the letters; bytes from 80h up by value), with `strcmp` deciding between names that differ only in case. The order is the same whatever order the directory holds them in (*tested*).
- **Operands keep their order.** With `LIBC16_GLOB_APPEND`, each call's paths follow those already there, so expanding a program's operands one after the other keeps the command line's order, and only each wildcard's own matches are sorted (*tested*).
- **Duplicates.** One search returns each directory entry once. Two operands that name the same file both add it (`*.TXT A.TXT` gives `A.TXT` twice), as a Unix shell would; a program that wants each file once removes them itself.

## Results and memory

- **`*g`.** After a call that returns 0, `g->gl_pathv[0]` to `g->gl_pathv[g->gl_pathc - 1]` are the paths, and `g->gl_pathv[g->gl_pathc]` is a null pointer, so `gl_pathv` can serve as an `argv` (for `spawnv`, say). When `gl_pathc` is 0, `gl_pathv` is a null pointer.
- **Without `LIBC16_GLOB_APPEND`** `*g` is set empty first, without freeing what it held: free an earlier result with `libc16_globfree` before reusing `*g`.
- **On failure** (any result but 0) no path is added: `*g` is as the call found it with `LIBC16_GLOB_APPEND`, or empty without, and nothing the call allocated is left allocated (*tested*, with the heap full and given back a few bytes at a time, so that `malloc` and `realloc` fail at many points of a search). Earlier paths stay, and are still freed with `libc16_globfree`.
- **`libc16_globfree(g)`** frees every path and the array and sets `*g` empty (`gl_pathc` 0, `gl_pathv` null). It may be called on an empty or all-zeros `*g`, and again after that.
- **Ownership.** The paths and the array are the glob's, from `malloc` and `realloc` (`docs/spec/heap.md`). The program may read them and change a path's characters, but frees them only through `libc16_globfree`.
- **Memory.** Each path is its own block, its length plus 1 bytes plus the heap's 2-byte header; a match in `DOCS\` costs `5 + 13 + 2` bytes at most. The array holds `gl_pathc + 1` pointers (2 bytes each in the small model, 4 with far data) and grows by doubling, from 8, with `realloc`, so it may have up to twice the slots it uses. A search of 300 matches with 15-character paths (`D:\BIG\N000.DAT`) holds 7,128 bytes of heap in the small model and 8,140 with far data under DOSBox-X: within 24 bytes a path and its array (*tested*). A search uses 43 bytes of stack for its DOS search record, which it keeps to itself: the disk transfer area is restored after each DOS call, so a search of the program's own with `_dos_findfirst` is not disturbed, and none is left open when `libc16_glob` returns.
- **Limits.** In the small model all of it is in the heap, inside `DGROUP`'s 64 KiB with the program's other data; with far data the heap can use the rest of conventional memory. The array is one block, so it has at most 32,767 pointers in the small model and 16,383 with far data; more is `LIBC16_GLOB_NOSPACE`.
- **No iterator.** Sorting needs every name, so there is no streaming form. A program that wants DOS's order without keeping the names uses `_dos_findfirst` and `_dos_findnext` itself.

## Files that change

The paths are a snapshot of the directory at the time of the call. A file deleted or renamed before the program opens it fails at the open, and a file created after the call is not in the list; a program reports such an open failure as for any other operand. A program that creates files in the directory it is expanding (writing `OUT.TXT` while expanding `*.TXT`) gets the list from before it wrote them.

## Example consumer

`tests/glob/gcat.c`, a minimal `cat`: each operand is expanded in order, an operand that matches nothing is reported, and the files are copied to standard output byte for byte.

```c
#include <fcntl.h>
#include <io.h>
#include <libc16.h>
#include <string.h>

int main(int argc, char **argv)
{
    libc16_glob_t g = { 0 };
    static char buf[512];
    unsigned i;
    int n, fd, status = 0;

    setmode(1, O_BINARY);       /* byte for byte */
    for (i = 1; i < (unsigned)argc; i++) {
        int r = libc16_glob(argv[i], LIBC16_GLOB_APPEND, &g);
        if (r == LIBC16_GLOB_NOMATCH)
            ... report "no match" for argv[i]; status = 1;
        else if (r != 0)
            ... report strerror(errno) for argv[i]; status = 1;
    }
    for (i = 0; i < g.gl_pathc; i++) {
        fd = open(g.gl_pathv[i], O_RDONLY | O_BINARY);
        ... copy fd to handle 1 with read and write; report a failed open
    }
    libc16_globfree(&g);
    return status;
}
```

## Linking

`libc16_glob` and `libc16_globfree` are separate members; a program that calls neither links neither, and nothing else in libc16 refers to them. `libc16_glob` links `_dos_findfirst`, `_dos_findnext`, `malloc`, `realloc`, `free`, `qsort`, `stricmp`, `strcmp`, `strlen`, `memcpy` and `errno`; `libc16_globfree`, `free`. `libc16_glob` is 697 bytes of code in the small model, 920 compact and 945 large, and `libc16_globfree` 55, 81 and 85. With what it links, a program that uses nothing else of these gains about 2.1 KB in the small model and 2.9 to 3.0 KB with far data; `docs/CONSUMERS.md` has the details.

## Constraints

8086 instructions only; plain C. Every model.
