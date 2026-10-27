/* ctype.h - libc16 (C89 7.3, in the "C" locale) */

#ifndef _LIBC16_CTYPE_H
#define _LIBC16_CTYPE_H

#include <_libc16.h>

int __cdecl isalnum(int c);
int __cdecl isalpha(int c);
int __cdecl iscntrl(int c);
int __cdecl isdigit(int c);
int __cdecl isgraph(int c);
int __cdecl islower(int c);
int __cdecl isprint(int c);
int __cdecl ispunct(int c);
int __cdecl isspace(int c);
int __cdecl isupper(int c);
int __cdecl isxdigit(int c);
int __cdecl tolower(int c);
int __cdecl toupper(int c);

#endif
