/* The handle table - libc16 (docs/spec/io.md) */

#include "io16.h"

unsigned char __libc16_fdflags[FD_MAX];
unsigned char __libc16_fdpend[FD_MAX];
