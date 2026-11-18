/* process.h - libc16 (Microsoft C's spawn functions, system and getpid,
 * the parts docs/spec/process.md implements) */

#ifndef _LIBC16_PROCESS_H
#define _LIBC16_PROCESS_H

#include <_libc16.h>

/* spawn modes: only P_WAIT is supported */
#define P_WAIT    0
#define P_NOWAIT  1
#define P_OVERLAY 2

int __cdecl getpid(void);
int __cdecl spawnl(int mode, const char *path, const char *arg0, ...);
int __cdecl spawnv(int mode, const char *path, char * const *argv);
int __cdecl spawnve(int mode, const char *path, char * const *argv, char * const *envp);
int __cdecl spawnvp(int mode, const char *path, char * const *argv);
int __cdecl spawnvpe(int mode, const char *path, char * const *argv, char * const *envp);
int __cdecl system(const char *cmd);

#endif
