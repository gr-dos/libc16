/* stdin - libc16 (docs/spec/stdio.md) */

#include <stdio.h>

static unsigned char buf[128];

FILE __cdecl __libc16_stdin = { buf, 0, buf, sizeof buf, 0, _IOREAD, 0 };
