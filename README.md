# libc16

A small C runtime for 16-bit MS-DOS programs built with Open Watcom C:
small memory model, 8086 instructions, DOS 2.0 and later. Each function
is its own library member, so a program links only what it calls, and
no Open Watcom runtime code is used (`docs/LICENSE-PROVENANCE.md`).
MIT licensed.

It provides what the `gr-dos/*16` utilities and GR-DOS's ATTRIB, MEM and
FDISK need (`docs/CONSUMERS.md`): the `.EXE` startup, `argc`/`argv`, `exit`
and `atexit`; string and memory functions, `memchr` included, with `memcpy`,
`memmove`, `memset` and `_fmemcpy` a word at a time;
`open`/`read`/`write`/`lseek` and friends with text mode, `isatty`, `dup`
and `dup2`, and `_doserrno` beside `errno`; `rename`, `unlink`, `remove`, `stat`, `fstat`, `getcwd`,
`chdir`, `mkdir` and `rmdir`; stdio streams with `fopen` and
`fdopen` (update modes included), `fgets`, `fputs`, `fseek`, `ftell`, `clearerr` and character
I/O, unbuffered unless the program asks for a buffer with `setvbuf` or
`setbuf`; `strtol`, `strtoul`, `qsort`, `rand`, `srand` and `abort`;
`setjmp` and `longjmp`; `system`, `libc16_systeme` (`system` with an environment), `spawnl`,
`spawnv`, `spawnve`, `spawnvp`, `spawnvpe` and `getpid`; `signal` and `raise`, with `SIGINT`
from Ctrl-C; `time`,
`gmtime`, `localtime` and `strftime`;
`malloc`/`free`; `_dos_findfirst`/`_dos_findnext`, `_dos_getftime`/`_dos_setftime`, `int86`/`int86x`,
`intdos`/`intdosx`, `segread`, `bdos`, `kbhit`, `getch`, `putch`, `getenv` and `_fullpath`; the compiler's 32- and 64-bit helpers; and
`libc16_ultoa`/`libc16_ltoa` in place of `printf`, which libc16 does not
have; and `libc16_glob`/`libc16_globfree`, opt-in wildcard expansion of
file operands. `docs/spec/` describes each function; `docs/ABI.md` is the
contract with the programs that link it.

## Using a release

Each release has `libc16-VERSION.zip` and `libc16-VERSION.zip.sha256`.
Check the archive before unpacking it:

    sha256sum -c libc16-0.01.zip.sha256
    unzip libc16-0.01.zip

It holds one directory:

| Path | What |
|---|---|
| `libc16-0.01/VERSION` | the version |
| `lib/LIBC16.LIB`, `lib/LIBC16C.LIB`, `lib/LIBC16L.LIB` | the library for the small, compact and large memory models |
| `include/` | the public headers |
| `docs/` | the ABI, design, provenance and per-function specs, and the similarity audit (`AUDIT.txt`) |
| `BUILD.txt` | the source revision, the pinned toolchain, the flags and how to rebuild |
| `tools/fetch-watcom` | downloads and checks the pinned Open Watcom snapshot (not included) |
| `LICENSE`, `README.md` | |

Compile with the Open Watcom snapshot named in `BUILD.txt`, with its
default register calling convention, and only libc16's headers; link
with no default libraries:

    wcc -zl -zld -x -ms -0 -s -bt=dos -Ilibc16-0.01/include hello.c
    wlink option nodefaultlibs format dos file hello.obj \
        library libc16-0.01/lib/LIBC16.LIB

For the compact model compile with `-mc` and link `LIBC16C.LIB`; for
the large model, `-ml` and `LIBC16L.LIB` (DESIGN.md, decision 11). The
medium and huge models are not provided.

`docs/ABI.md`, "Consumer build", explains each flag. On Linux,
`libc16-0.01/tools/fetch-watcom DIR` downloads the snapshot into `DIR`,
checks its SHA-256 and prints the directory to use as `WATCOM` (run
`$WATCOM/binl64/wcc`). On DOS, the same snapshot's DOS-hosted `wcc.exe`
and `wlink.exe` (in `binw`, under DOS/4GW, so a 386 or later) take the
same options; that use is not tested.

### In a `gr-dos/*16` utility

Utilities fetch a pinned release with `scripts/fetch-libc16`
(`docs/CONSUMERS.md`, "Using libc16"). It downloads the exact versioned
ZIP, checks it against the SHA-256 written in the script before
unpacking it, and unpacks it to `build/libc16`. The `Makefile` then
reads the library and headers from `$(LIBC16_ROOT)`, which is
`build/libc16` by default. To build against a local checkout instead
(built with `make`), set `LIBC16_ROOT` to it:

    make LIBC16_ROOT=../libc16

A checkout's `make` stages the same layout in `build/stage`, which is
what such a build uses.

## Building from source

On Linux, with Perl 5.42 as `perl`, a C compiler and make (to build
JWasm), git, curl and xz, and `dosbox-x` for the tests:

    make            # build/s/LIBC16.LIB, build/c/LIBC16C.LIB and
                    # build/l/LIBC16L.LIB, staged with the headers in build/stage
    make test       # the ABI, link-map, 8086 and selective-linking checks,
                    # and the test programs under DOSBox-X, in every model
    make audit      # the similarity audit of each library
    make MODEL=c test   # lib, test or audit for one model (s, c or l)
    make dist       # build/dist/libc16-VERSION.zip and its .sha256, and make src
    make src        # build/dist/libc16-VERSION-src.tar.gz and its .sha256
    make install PREFIX=DIR   # copy build/stage to DIR

The pinned Open Watcom snapshot and JWasm are fetched into
`build/tools` (DESIGN.md, decisions 1 and 4).

## Releasing

`VERSION` holds the version, DOS style (0.01, 0.02, ... 1.00), not
semantic versioning. When a pull request that changes it is merged to
`main`, the release workflow (`.github/workflows/release.yml`) tags the
merge commit `v<VERSION>`. It then builds, tests and audits, packages
and checks the ZIP and the source archive, and publishes a GitHub
release with both and their checksums. A push to `main` whose `VERSION`
already has a tag publishes no release. Running the workflow by hand on
`main` does the same check, which bootstraps or retries a release whose
tag does not exist yet.

### Source archive

Each release also has `libc16-VERSION-src.tar.gz` and its `.sha256`: the
complete source, for programs that link libc16 into a program whose
license requires its corresponding source, such as the GPL (GR-DOS's
`dist16.source` `depends`). `scripts/make-src` makes it with `git archive`
from the tagged commit, never from a working tree, under the single
directory `libc16-VERSION/`: everything committed at the tag, with
`LICENSE`, `README.md` and the `Makefile`. It is the same for the same
commit, and `git get-tar-commit-id` reads the commit from it. To build it,
unpack it and run `make` as above. BUILD.txt then shows the revision as
`unknown`, since there is no `.git`, and the libraries differ from the
release's only in the source paths `wcc` records.

`scripts/check-src` checks an archive against its commit: the
checksum, the recorded commit, the top directory, and every file and its
executable bit, with nothing missing or added. `make src` (and so
`make dist`, in CI) makes and checks one for `HEAD`. On every run, the
release workflow gives any release without a source archive one made
from its tag, which is how v0.01 to v0.07 got theirs. It never replaces
an existing asset. It then downloads and checks every release's source
archive.

## License

MIT; see `LICENSE`. The build tools are not included: Open Watcom and
JWasm are under the Sybase Open Watcom Public License.
