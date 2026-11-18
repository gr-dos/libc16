# Spec: signals

**Header:** `<signal.h>`  
**Members:** `signal`, `raise`, each its own member; private members `sigtab` (the dispositions) and `sigint` (the INT 23h handler, in assembly); `abort` (`docs/spec/stdlib.md`) changes. In each of the small, compact and large libraries (decision 11)  
**Tests:** `tests/signal` (run by `tests/run-dos`, in every model)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation. `sigint` is written in assembly: it is an interrupt handler, entered with the registers of whatever DOS call was running, which C cannot express.

## Sources

- C89 7.7 (`<signal.h>`, `signal`, `raise`, `sig_atomic_t`, `SIG_DFL`, `SIG_IGN`, `SIG_ERR`, the six signal names) and 7.10.4.1 (`abort` and `SIGABRT`).
- The Microsoft C 5.1 run-time library reference, for the signal numbers (`SIGINT` 2, `SIGILL` 4, `SIGFPE` 8, `SIGSEGV` 11, `SIGTERM` 15, `SIGABRT` 22) and the values of `SIG_DFL` (0), `SIG_IGN` (1) and `SIG_ERR` (-1).
- INT 23h (Ctrl-C handler: entered with the registers of the interrupted DOS call; `IRET` resumes the call; a handler may call any DOS function), INT 21h functions 25h and 35h (set and get an interrupt vector), the INT 23h vector in the PSP at offset 0Eh that DOS restores when a program ends, and which DOS functions check for Ctrl-C (01h to 0Ch, except 06h and 07h, always; the others only with `BREAK ON`): Ralf Brown's Interrupt List; MS-DOS 2.0 `SYSCALL.DOC`. INT 16h function 05h (store a keystroke), used only by the tests.
- `docs/ABI.md`: the `__cdecl` convention, callbacks (decision 2), and `SS` = `DGROUP` in every model.
- gr-dos/perl16's `docs/M3-RUNTIME-AUDIT.md`, sections 6.6 and 7.4, for the proposed contract.
- Our own experiments under DOSBox-X (`tests/run-dos`), with a Ctrl-C stored with INT 16h 05h:
  - function 08h reads it, writes `^C` and CR LF to standard output, issues INT 23h, and when the handler returns with `IRET`, returns 3 in `AL` (MS-DOS documents that the call is restarted instead);
  - functions 07h, 0Bh, 02h, 30h, 19h and 40h to the console do not check for it, with or without `BREAK ON` (33h);
  - a program that issues INT 23h itself with DOSBox-X's own handler in place does not terminate cleanly: the session stops running the batch, even when the program is a child run with `spawnl`. So the tests never reach DOS's own INT 23h handler.

## `<signal.h>`

- `typedef int sig_atomic_t;`
- `SIG_DFL` `((void (__cdecl *)(int))0)`, `SIG_IGN` `((void (__cdecl *)(int))1)`, `SIG_ERR` `((void (__cdecl *)(int))-1)`.
- `SIGINT` 2, `SIGILL` 4, `SIGFPE` 8, `SIGSEGV` 11, `SIGTERM` 15, `SIGABRT` 22, and `NSIG` 32 (one more than the largest signal number).
- `void (__cdecl *signal(int sig, void (__cdecl *func)(int)))(int);` and `int raise(int sig);`, both `__cdecl`.

A handler is a `__cdecl` function taking the signal number (decision 2's callback rule); a near function in the small and compact models, far in the large.

## Signal numbers and dispositions

Every number from 1 to 31 is a signal. Each has a disposition: `SIG_DFL` (the initial one, for every signal), `SIG_IGN`, or a handler. Only `SIGINT` (from INT 23h) and `SIGABRT` (from `abort`) are ever delivered other than by `raise`; the rest exist so that a program can set and raise them (Perl sets all 31 to `SIG_DFL`).

## `signal(int sig, func)`

Sets the disposition of `sig` to `func` and returns the previous one. A `sig` outside 1 to 31, or a `func` of `SIG_ERR`, changes nothing and returns `SIG_ERR` with `errno` `EINVAL`. `errno` is unchanged on success.

For `SIGINT`, the first call that sets `SIG_IGN` or a handler installs libc16's INT 23h handler (`sigint`), after saving the vector it replaces (INT 21h 35h and 25h). It is installed once and stays installed; a later `SIG_DFL` only changes the disposition. Since DOS restores INT 23h from the PSP when the program ends, the program leaves the vector as it found it.

## `int raise(int sig)`

Delivers `sig` and returns 0; a `sig` outside 1 to 31 returns a non-zero value with `errno` `EINVAL`.

- **`SIG_IGN`:** nothing happens.
- **A handler:** the disposition is set to `SIG_DFL` first (C89 allows this for every signal), then the handler is called with `sig`; when it returns, `raise` returns 0. The handler may set a disposition again, `raise` another signal, call `exit`, or `longjmp` out.
- **`SIG_DFL`:** the program ends with exit code 3 (INT 21h 4Ch), as Microsoft C's default action does, without calling the `atexit` functions or writing anything. This is so for `SIGINT` too: `raise` does not issue INT 23h.

## INT 23h (`SIGINT`)

DOS issues INT 23h when it finds a Ctrl-C (or Ctrl-Break) during a DOS call that checks for one (see "Sources"): so `SIGINT` arrives only during such a call, never in the middle of other code. libc16's handler, by the disposition of `SIGINT` at that moment:

- **`SIG_IGN`:** returns with `IRET`, changing no register or flag, so DOS resumes the call and the Ctrl-C is ignored.
- **A handler:** sets the disposition to `SIG_DFL`, then saves every register it may change, sets `DS` to `DGROUP`, enables interrupts (`sti`), clears the direction flag, and calls the handler with `SIGINT`, on the stack DOS returned to (the program's own). When the handler returns, it restores the registers and returns with `IRET`, so DOS resumes the interrupted call with the registers and flags it had.
- **`SIG_DFL`:** jumps to the vector it saved when it was installed, with every register and the stack as on entry, so DOS's own action (normally ending the program) follows.
- If `SS` is not `DGROUP`, the Ctrl-C did not interrupt this program's own code but a child it is running (`docs/spec/process.md`; the child inherits the vector): a handler is not called, and the handler jumps to the saved vector as for `SIG_DFL`, leaving the disposition unchanged. `SIG_IGN` still returns with `IRET`.

What a handler may do: anything a `raise` handler may, including DOS calls (INT 23h is issued when DOS can be called again). A `longjmp` out of the handler abandons the interrupted DOS call; interrupts stay enabled.

## `abort`

`abort` (`docs/spec/stdlib.md`) first delivers `SIGABRT` if a handler is set for it: it sets the disposition to `SIG_DFL` and calls the handler with `SIGABRT`. If the handler returns, or the disposition is `SIG_IGN` or `SIG_DFL`, `abort` goes on as before: it writes its message and ends the program with exit code 3. A handler that does not return (a `longjmp` out, or `exit`) keeps the program running or ending its own way, as C89 7.10.4.1 allows.

`abort` must still link no other member and not need the startup. It learns of a handler through a communal variable, `___libc16_sigabrt`, a code pointer (2 bytes in the small and compact models, 4 in the large, 0 unless set) that `sigtab` also declares and `signal` sets: `abort` calls it, with no arguments, if it is not 0. A program that does not call `signal` has nothing more than before but that variable and the test of it.

## Constraints

- All are `__cdecl` (`docs/ABI.md`). 8086 instructions only.
- `raise` links `sigtab` and not `signal` or `sigint`; `signal` links `sigtab` and `sigint`. Neither links the startup, the heap or stdio.
- The dispositions are in BSS (`sigtab`): 31 code pointers.
- In the small and compact models a handler is a code offset, which must not be 0 (`SIG_DFL`) or 1 (`SIG_IGN`): C89 7.7 requires both to compare unequal to any function's address. The startup's code reserve guarantees it (`docs/spec/startup.md`, "Null function pointer"); `sigtab` has none of its own. A program without the startup must keep its handlers off those offsets itself.

## Tests

`tests/signal`, in every model:

- `signal`: every number from 1 to 31 starts at `SIG_DFL` and returns the previous disposition when set; 0, 32, -1 and `NSIG` give `SIG_ERR` and `EINVAL`, as does `SIG_ERR` as `func`; `errno` unchanged on success. `raise` with `SIG_IGN` (nothing happens) and with a handler (called once with the number, disposition reset to `SIG_DFL` first, a handler that sets itself again, one that raises another signal, one that `longjmp`s out); `raise` of 0 and 32 fails. INT 23h issued by the test (`sigasm.asm`) with a handler, with `SIG_IGN`, and with `SIG_DFL` after a handler has run: the test first sets INT 23h to its own handler, which `signal` saves and `SIG_DFL` must jump to. Through each: `SI`, `DI`, `BP`, `ES`, `DS` (set to something other than `DGROUP`) and the direction flag are unchanged after the `IRET`; in the handler `DS` and `SS` are `DGROUP`, interrupts are enabled and the direction flag is clear. A `longjmp` out of a handler called through INT 23h. INT 23h issued with `SS` not `DGROUP` (the same memory, `SS` one paragraph higher): the test's own handler runs, the C handler does not, and the disposition is unchanged. A Ctrl-C stored with INT 16h 05h and read with `getch`: the handler is called through DOS.
- `sigdfl`, run by the cases with `term` and with `int`: `raise(SIGTERM)` and `raise(SIGINT)` with `SIG_DFL` end the program with exit code 3, without calling an `atexit` function.
- `sigabrt`, run with `ret`, `ign` and `jump`: a `SIGABRT` handler that returns (exit code 3, handler called once), `SIG_IGN` (exit code 3) and a handler that `longjmp`s out (the program continues, exit code 0).
- `sigvec`: runs `SIGKID.EXE` (`sigkid.c`, which sets a `SIGINT` handler and exits) with `spawnl`, then checks that INT 23h is the vector it was before.
- `tests/stdlib/abort` still links only `abort` and what it linked before (`tests/stdlib/members`).
