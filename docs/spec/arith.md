# Spec: compiler arithmetic helpers

**Members:** one per symbol below, or one per closely related group (a quotient and remainder pair may share code), so that a program links only the operations it uses  
**Tests:** `tests/arith` (run by `tests/run-dos`), `tests/check-abi`

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation.

## Sources

- Which operations call which symbol: call-site experiments, compiling one-line C functions with the consumer flags of `docs/ABI.md` and listing the `EXTRN` records of our own objects with `wdis` (table below).
- The register convention of each symbol: the same experiments, reading the disassembly of our own call sites (which registers the caller loads before the call, which it reads after, and which it keeps live across it).
- C99 6.5.5 (multiplicative operators: the quotient is truncated toward zero, and `(a/b)*b + a%b == a`), 6.5.7 (shift operators), 6.2.5 (unsigned arithmetic is modulo 2^N).

## Symbols

| Symbol | Operation | Operand type |
|---|---|---|
| `__I4M` | `a * b` | `long` |
| `__U4M` | `a * b` | `unsigned long` |
| `__I4D` | `a / b` and `a % b` | `long` |
| `__U4D` | `a / b` and `a % b` | `unsigned long` |
| `__I8M` | `a * b` | `long long` |
| `__U8M` | `a * b` | `unsigned long long` |
| `__I8DQ` | `a / b` | `long long` |
| `__I8DR` | `a % b` | `long long` |
| `__U8DQ` | `a / b` | `unsigned long long` |
| `__U8DR` | `a % b` | `unsigned long long` |
| `__I8LS` | `a << n` | `long long` |
| `__U8LS` | `a << n` | `unsigned long long` |
| `__I8RS` | `a >> n` (arithmetic) | `long long` |
| `__U8RS` | `a >> n` (logical) | `unsigned long long` |

The compiler also calls these for the compound assignments (`*=`, `/=`, `%=`, `<<=`, `>>=`) and for constant operands. `long` shifts, additions, subtractions and comparisons, and `long long` additions, subtractions and comparisons, are inline and need nothing.

## Must

- Give the result C requires for every operand value where C defines one: products modulo 2^32 or 2^64, quotients truncated toward zero, remainders with the sign of the dividend, shifts by 0 to 31 (`long`) or 0 to 63 (`long long`). For `__I4D` and `__U4D`, the caller reads both the quotient and the remainder from wherever the convention puts them.
- Where C leaves the result undefined (division by zero, `LONG_MIN / -1`, `LLONG_MIN / -1`, shift counts out of range), do anything except corrupt memory or registers the convention says are preserved; a divide-error trap (INT 0) is acceptable.
- Follow the register convention the compiler's call sites show exactly: arguments where the caller puts them, results where the caller reads them, and every other register the caller keeps live across the call preserved. When an experiment cannot show whether a register must be preserved, preserve it.
- Record each convention in `docs/ABI.md`'s compiler support table, with the experiment it came from.
- Use 8086 instructions only (`scripts/check-8086`), and keep each operation small: these are linked into programs that care about size.

## Tests

`tests/arith` runs each operation under DOSBox-X on operands that exercise every path: zero, one, all-ones, the signed extremes, every sign combination, divisors with a zero and a non-zero high word, and dividends smaller than divisors. Expected results are computed on the host. `tests/check-abi`'s arithmetic fixture lists every symbol above.
