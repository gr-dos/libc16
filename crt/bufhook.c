/* The hooks of buffered output - libc16 (docs/spec/stdio.md,
 * "Buffering"). Data only: the stream functions link this member, and
 * setvbuf and setbuf, through the engine, set the pointers. */

#include <stddef.h>
#include "stdio16.h"

unsigned (*__libc16_bufput)(FILE *fp, const void *p, unsigned n);
int (*__libc16_bufctl)(FILE *fp, int how);
