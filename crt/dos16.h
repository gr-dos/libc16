/* Private to libc16: directory searches (docs/spec/dos.md). */

#ifndef LIBC16_DOS16_H
#define LIBC16_DOS16_H

#include <dos.h>

/* Runs INT 21h function fn (4Eh find first, 4Fh find next) with the disk
 * transfer area at f; returns 0 or the DOS error code, with errno set. */
unsigned __libc16_find(unsigned char fn, const char *path, unsigned attr, struct find_t *f);

#endif
