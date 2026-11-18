/* setjmp.h - libc16 (C89 7.6; docs/spec/setjmp.md) */

#ifndef _LIBC16_SETJMP_H
#define _LIBC16_SETJMP_H

#include <_libc16.h>

/* The return address (offset, then segment), SP after the return, BP,
 * SI, DI, DS and a reserved word: docs/ABI.md, "setjmp and longjmp" */
#define _JBLEN 8
typedef int jmp_buf[_JBLEN];

int  __cdecl setjmp(jmp_buf env);
void __cdecl longjmp(jmp_buf env, int val);

#endif
