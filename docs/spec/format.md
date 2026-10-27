# Spec: number formatting and conversion

**Headers:** `<libc16.h>`, `<stdlib.h>`  
**Members:** `ultoa` (`libc16_ultoa`), `ltoa` (`libc16_ltoa`), `strtoul`, `atoi`, in each of the small, compact and large libraries (decision 11)  
**Tests:** `tests/format` (run by `tests/run-dos`, in every model)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation. Decision 3 prefers the small helpers `libc16_ultoa` and `libc16_ltoa` to `printf` for simple formatting; they are libc16's own interface, not a standard one, so their names have the `libc16_` prefix. `strtoul` and `atoi` are C89's (7.10.1). `strftime` is in `docs/spec/time.md`.

## `char *libc16_ultoa(unsigned long value, char *buf, int radix)`

- Writes `value` in base `radix` (2 to 36) to `buf` as a null-terminated string and returns `buf`.
- Digits are `0`–`9` then lowercase `a`–`z`. There are no leading zeros, sign or prefix; zero is `"0"`.
- `buf` must hold 33 bytes for any value (32 binary digits and the terminator).
- If `radix` is outside 2 to 36, stores `""` and returns `buf`.

## `char *libc16_ltoa(long value, char *buf, int radix)`

- As `libc16_ultoa`, except that a negative `value` is written as `-` followed by its magnitude, in every radix. `LONG_MIN` is written correctly.
- `buf` must hold 34 bytes.

## `unsigned long strtoul(const char *nptr, char **endptr, int base)`

C89 7.10.1.6, in the "C" locale:

- Skips leading white space (space, `\t`, `\n`, `\v`, `\f`, `\r`), then accepts an optional `+` or `-`, then digits in `base`: `0`–`9` and the letters `a`–`z` or `A`–`Z` for 10 to 35, each less than `base`.
- `base` 2 to 36 reads that base; with `base` 16 an optional `0x` or `0X` may precede the digits. `base` 0 reads hexadecimal after `0x` or `0X`, octal after a leading `0`, and decimal otherwise.
- With a `-`, the result is the negation of the value read, in `unsigned long` arithmetic.
- If the value read is greater than `ULONG_MAX`, returns `ULONG_MAX` and sets `errno` to `ERANGE` (for `-` too). Otherwise `errno` is not changed.
- If `endptr` is not null, stores in `*endptr` a pointer to the first character after the number. If there is no number (no digits after the white space, sign and prefix), returns 0 and stores `nptr`; `"0x"` with no hexadecimal digit after it is the number `0` followed by `x`.
- `base` 1, or less than 0, or greater than 36: returns 0, stores `nptr` in `*endptr`, and sets `errno` to `EINVAL`.

## `int atoi(const char *nptr)`

C89 7.10.1.2: the value of the initial decimal number in `nptr`, read as `strtol(nptr, NULL, 10)` would (white space, then an optional sign, then decimal digits), converted to `int`; 0 if there is none. `errno` is not changed. When the value does not fit in `int` the result is undefined in C89; libc16 returns the low 16 bits of the value read modulo 2^32.

## Notes

- All are `__cdecl` (`docs/ABI.md`). A program that uses only one links only that one; any of them may link the 32-bit multiply and divide helpers (`docs/spec/arith.md`). `strtoul` and `atoi` do not link each other or the `<ctype.h>` table.
- Padding and field widths are left to the caller.
