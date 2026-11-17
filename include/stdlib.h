/* stdlib.h - libc16 (C89 7.10, the parts libc16 implements, and
 * Microsoft C's _fullpath and _MAX_PATH) */

#ifndef _LIBC16_STDLIB_H
#define _LIBC16_STDLIB_H

#include <_libc16.h>

#ifndef _LIBC16_SIZE_T
#define _LIBC16_SIZE_T
typedef unsigned size_t;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

/* The largest value rand returns */
#define RAND_MAX 32767

/* Longest full path, with its terminator: drive, colon, a 64-byte
 * directory and a separator and 8.3 name. */
#define _MAX_PATH 80

void          __cdecl abort(void);
int           __cdecl atexit(void (__cdecl *func)(void));
int           __cdecl atoi(const char *nptr);
void *        __cdecl calloc(size_t n, size_t size);
void          __cdecl exit(int status);
void          __cdecl free(void *p);
char *        __cdecl _fullpath(char *buf, const char *path, size_t size);
char *        __cdecl getenv(const char *name);
void *        __cdecl malloc(size_t size);
void          __cdecl qsort(void *base, size_t nmemb, size_t size,
                            int (__cdecl *compar)(const void *, const void *));
int           __cdecl rand(void);
void *        __cdecl realloc(void *p, size_t size);
void          __cdecl srand(unsigned seed);
long          __cdecl strtol(const char *nptr, char **endptr, int base);
unsigned long __cdecl strtoul(const char *nptr, char **endptr, int base);

#endif
