/* conio.h - libc16 (Microsoft C's console interface, the parts
 * docs/spec/dos.md implements) */

#ifndef _LIBC16_CONIO_H
#define _LIBC16_CONIO_H

#include <_libc16.h>

int __cdecl kbhit(void);
int __cdecl getch(void);
int __cdecl putch(int c);

#endif
