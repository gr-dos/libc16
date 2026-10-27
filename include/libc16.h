/* libc16.h - libc16's own helpers (docs/spec/format.md) */

#ifndef _LIBC16_LIBC16_H
#define _LIBC16_LIBC16_H

#include <_libc16.h>

char * __cdecl libc16_ltoa(long value, char *buf, int radix);
char * __cdecl libc16_ultoa(unsigned long value, char *buf, int radix);

#endif
