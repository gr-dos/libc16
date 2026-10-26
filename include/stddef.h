/* stddef.h - libc16 (C89 7.1.6) */

#ifndef _LIBC16_STDDEF_H
#define _LIBC16_STDDEF_H

#ifndef _LIBC16_SIZE_T
#define _LIBC16_SIZE_T
typedef unsigned size_t;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef int ptrdiff_t;
typedef unsigned short wchar_t;

#define offsetof(type, member) ((size_t)&((type *)0)->member)

#endif
