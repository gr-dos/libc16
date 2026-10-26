/* Private interface between the startup (cstart.asm) and its optional
 * members. Not installed. */

#ifndef LIBC16_STARTUP_H
#define LIBC16_STARTUP_H

extern unsigned _psp;           /* PSP segment */
extern int __libc16_argc;       /* passed to main */
extern char **__libc16_argv;

void __libc16_args(void);

#endif
