/* Test-only output to standard output (handle 1) through INT 21h function
 * 40h, so that startup tests need nothing from libc16 but the startup. */

#ifndef TESTIO_H
#define TESTIO_H

/* A data pointer that a test's pragma passes to DOS in DS:DX: the offset
 * in the small model, and the segment too with far data (docs/ABI.md,
 * "Memory models"). */
#if defined(__COMPACT__) || defined(__LARGE__)
#define T_DS_DX [ds dx]
#else
#define T_DS_DX [dx]
#endif

void t_puts(const char *s);
void t_putu(unsigned u);

#endif
