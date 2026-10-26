/* Private interface between the libc16 startup members (docs/spec/startup.md).
 * The variables are defined in crt/cstart.asm. */

#ifndef LIBC16_CRT_STARTUP_H
#define LIBC16_CRT_STARTUP_H

extern unsigned _psp;           /* PSP segment */
extern int __libc16_argc;       /* passed to main as argc */
extern char **__libc16_argv;    /* passed to main as argv */

void __libc16_args(void);

#endif
