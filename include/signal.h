/* signal.h - libc16 (C89 7.7, with Microsoft C's signal numbers;
 * docs/spec/signal.md) */

#ifndef _LIBC16_SIGNAL_H
#define _LIBC16_SIGNAL_H

#include <_libc16.h>

typedef int sig_atomic_t;

#define SIG_DFL ((void (__cdecl *)(int))0)
#define SIG_IGN ((void (__cdecl *)(int))1)
#define SIG_ERR ((void (__cdecl *)(int))-1)

#define SIGINT   2
#define SIGILL   4
#define SIGFPE   8
#define SIGSEGV  11
#define SIGTERM  15
#define SIGABRT  22

/* One more than the largest signal number */
#define NSIG     32

int __cdecl raise(int sig);
void (__cdecl * __cdecl signal(int sig, void (__cdecl *func)(int)))(int);

#endif
