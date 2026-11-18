# Spec: non-local jumps

**Header:** `<setjmp.h>`  
**Members:** `setjmp`, `longjmp`, each its own member, in each of the small, compact and large libraries (decision 11)  
**Tests:** `tests/setjmp` (run by `tests/run-dos`, in every model)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation. Like the startup, these are written in assembly: they save and restore the caller's registers and stack, which C cannot express.

## Sources

- C89 7.6 (`setjmp`, `longjmp`, `jmp_buf`).
- The Microsoft C 5.1 run-time library reference, for `<setjmp.h>` and the names.
- `docs/ABI.md`: the `__cdecl` convention, the registers a callee preserves (`SI`, `DI`, `BP`, `DS`, `SS`), the memory models, and the experiments in "setjmp and longjmp" there.
- gr-dos/perl16's `docs/M3-RUNTIME-AUDIT.md`, sections 6.3 and 7.3, for the proposed contract.

## `jmp_buf`

`typedef int jmp_buf[8];`: 16 bytes in every model. `setjmp` writes all 8 words:

| Word | Offset | Holds |
|---|---|---|
| 0 | 0 | the return address's offset |
| 1 | 2 | the return address's segment in the large model; in the small and compact models, `CS` |
| 2 | 4 | `SP` as it is after `setjmp` returns: its value on entry plus the return address (2 bytes, or 4 in the large model) |
| 3 | 6 | `BP` |
| 4 | 8 | `SI` |
| 5 | 10 | `DI` |
| 6 | 12 | `DS` |
| 7 | 14 | 0 (reserved) |

The layout is part of the ABI (`docs/ABI.md`), so that a program may inspect it, but programs should not change it. `SS` is not saved: it is `DGROUP` in every model (`docs/spec/startup.md`).

## `int setjmp(jmp_buf env)`

Saves the calling environment in `env` and returns 0 (C89 7.6.1.1). `setjmp` is a function, not a macro (C89 allows either), `__cdecl`, symbol `_setjmp`, called near in the small and compact models and far in the large model. `env` is a near pointer in the small model and a far one in the compact and large models, so with far data `env` need not be in `DGROUP`. It changes no register the `__cdecl` convention preserves, and only `AX` (and the flags) of the rest, besides `env`'s words.

C89 lets a program call `setjmp` only as the whole of an expression statement, or as the controlling expression of an `if`, `switch` or loop, alone, negated, or compared with an integer constant; libc16 adds no further use.

## `void longjmp(jmp_buf env, int val)`

Restores the environment `env` saved, and returns from that call of `setjmp` again, with the value `val`, or 1 if `val` is 0 (C89 7.6.2.1). It does not return to its caller. `__cdecl`, symbol `_longjmp`, called as `setjmp` is.

On the return through `setjmp`:

- `SP`, `BP`, `SI`, `DI` and `DS` are as `env` holds them: as they were when `setjmp` returned the first time. `SS` is unchanged.
- `AX` is the value; `BX`, `CX`, `DX` and `ES` are unspecified.
- The direction flag is clear, as C code expects, whatever it was when `longjmp` was called. Interrupts are as `longjmp` found them.
- `longjmp` reads all of `env` before it changes `SP`, so `env` may be anywhere: in `DGROUP`, in a far segment, or on the stack in the frame it returns to.
- In the small model `longjmp` may rely on `DS` = `DGROUP` on entry, as any `__cdecl` function may; in the compact and large models it may not, and reaches `env` through its far pointer.

So, as C89 says, objects of static duration have the values they had when `longjmp` was called, and so do automatic objects of the function that called `setjmp` that are `volatile` or were not changed between the two calls. A non-`volatile` automatic object changed in between is indeterminate: the compiler may keep it in `SI` or `DI`, which `longjmp` restores to their earlier value (`docs/ABI.md`).

**Undefined, as in C89:** `env` not filled by `setjmp`; the function that called `setjmp` having returned since; and a `setjmp` used other than as above. libc16 does not detect these. Jumping out of a callback that a libc16 function called (`qsort`'s comparison, an `atexit` function) abandons that function's work but is otherwise as any other jump: no libc16 function holds a resource across a callback that the jump would leak.

## Constraints

- Assembly, from one source per member for the three models (`crt/model.inc`). 8086 instructions only (`scripts/check-8086`).
- Neither references any other member, nor needs the startup (`docs/spec/startup.md`, "Programs without the startup"); `setjmp` does not link `longjmp`, nor `longjmp` `setjmp`.

## Tests

`tests/setjmp/setjmp` (with `tests/setjmp/sjasm.asm`), in every model:

- the direct return of 0, then the values 5, -1, 32767, and 1 for `longjmp(env, 0)`;
- a jump out of 20 nested calls (far calls in the large model), each frame with a local array, and a jump out of `qsort`'s comparison function;
- `volatile` locals changed after `setjmp`, a non-`volatile` local not changed, and static variables, after the jump;
- 1,000 jumps to the same buffer, with `SP` the same at the return each time;
- `env` in `DGROUP`, on the stack, and in the compact and large models in a far segment outside `DGROUP`;
- `sj_regs` (assembly) calls `setjmp` with marker values in `SI`, `DI` and `BP`, changes them, and calls `longjmp`: they, `SP` and the value must be back;
- `sj_dirty` (assembly) calls `longjmp` with the direction flag set, `BX`, `CX`, `DX`, `SI`, `DI`, `BP` and `ES` changed and, with far data, `DS` = 0: after the jump `DS` is `DGROUP` and the direction flag is clear.
