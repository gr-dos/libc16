# Spec: number formatting helpers

**Header:** `<libc16.h>`  
**Members:** `ultoa` (`libc16_ultoa`), `ltoa` (`libc16_ltoa`)  
**Tests:** `tests/format` (run by `tests/run-dos`)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation. Decision 3 prefers these small helpers to `printf` for simple formatting; they are libc16's own interface, not a standard one, so their names have the `libc16_` prefix.

## `char *libc16_ultoa(unsigned long value, char *buf, int radix)`

- Writes `value` in base `radix` (2 to 36) to `buf` as a null-terminated string and returns `buf`.
- Digits are `0`–`9` then lowercase `a`–`z`. There are no leading zeros, sign or prefix; zero is `"0"`.
- `buf` must hold 33 bytes for any value (32 binary digits and the terminator).
- If `radix` is outside 2 to 36, stores `""` and returns `buf`.

## `char *libc16_ltoa(long value, char *buf, int radix)`

- As `libc16_ultoa`, except that a negative `value` is written as `-` followed by its magnitude, in every radix. `LONG_MIN` is written correctly.
- `buf` must hold 34 bytes.

## Notes

- Both are `__cdecl` (`docs/ABI.md`). A program that uses only one links only that one; either may link the 32-bit divide helper (`docs/spec/arith.md`).
- Padding and field widths are left to the caller.
