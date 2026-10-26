/* Test-only output to standard output (handle 1) through INT 21h function
 * 40h, so that startup tests need nothing from libc16 but the startup. */

#ifndef TESTIO_H
#define TESTIO_H

void t_puts(const char *s);
void t_putu(unsigned u);

#endif
