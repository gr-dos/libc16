/* string.h - libc16 (C89 7.11, and Microsoft C's strdup, stricmp, strlwr) */

#ifndef _LIBC16_STRING_H
#define _LIBC16_STRING_H

#ifndef _LIBC16_SIZE_T
#define _LIBC16_SIZE_T
typedef unsigned size_t;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

int    __cdecl memcmp(const void *s1, const void *s2, size_t n);
void * __cdecl memcpy(void *dst, const void *src, size_t n);
void * __cdecl memset(void *s, int c, size_t n);
char * __cdecl strcat(char *dst, const char *src);
char * __cdecl strchr(const char *s, int c);
int    __cdecl strcmp(const char *s1, const char *s2);
char * __cdecl strcpy(char *dst, const char *src);
char * __cdecl strdup(const char *s);
char * __cdecl strerror(int errnum);
int    __cdecl stricmp(const char *s1, const char *s2);
size_t __cdecl strlen(const char *s);
char * __cdecl strlwr(char *s);
int    __cdecl strncmp(const char *s1, const char *s2, size_t n);
char * __cdecl strrchr(const char *s, int c);

#endif
