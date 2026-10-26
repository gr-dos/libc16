/* stdlib.h - libc16 (C89 7.10, and Microsoft C's _fullpath and _MAX_PATH) */

#ifndef _LIBC16_STDLIB_H
#define _LIBC16_STDLIB_H

#ifndef _LIBC16_SIZE_T
#define _LIBC16_SIZE_T
typedef unsigned size_t;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

/* Longest full path, with its terminator: drive, colon, a 64-byte
 * directory and a separator and 8.3 name. */
#define _MAX_PATH 80

void   __cdecl free(void *p);
char * __cdecl _fullpath(char *buf, const char *path, size_t size);
char * __cdecl getenv(const char *name);
void * __cdecl malloc(size_t size);

#endif
