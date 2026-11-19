/* Private to libc16: the cores of the spawn functions, system and
 * libc16_systeme (docs/spec/process.md). */

#ifndef LIBC16_PROC16_H
#define LIBC16_PROC16_H

#include <stddef.h>

/* The buffer for a command tail: its length byte, at most 126 bytes and
 * the carriage return; and for a program name, terminator included. */
#define TAIL_MAX 126
#define EXEC_NAME_MAX 128

/* Runs name with INT 21h 4B00h, with the environment segment env (0: the
 * parent's), the command tail tail (length byte first) and a blank FCB
 * for both FCBs, and returns the child's exit code (0 to 255), or minus
 * the DOS error code, with errors 3 (path not found) and 15 (invalid
 * drive) returned as 2 (file not found) (crt/dosexec.asm). */
int __cdecl __libc16_dosexec(const char *name, unsigned env, const char *tail);

/* Runs one name: returns as __libc16_dosexec does. */
typedef int (*__libc16_try)(const char *name, unsigned env, const char *tail);

/* Runs name, with .COM then .EXE if its last component has no extension
 * (crt/spawnx.c). */
int __libc16_spawnext(const char *name, unsigned env, const char *tail);

/* Runs name as __libc16_spawnext does, then through PATH (crt/spawnp.c). */
int __libc16_spawnp(const char *name, unsigned env, const char *tail);

/* The spawn functions: checks mode and argv, builds the command tail and,
 * for a non-null envp, the environment block, and runs path with try. */
int __libc16_spawnx(int mode, const char *path, char * const *argv,
    char * const *envp, __libc16_try try);

/* Builds the environment block from envp (not null): each string, then
 * one more zero, in memory from INT 21h 48h. Returns its segment, or 0
 * with errno E2BIG (over 32767 bytes) or ENOMEM (crt/envblock.c). */
unsigned __libc16_envblock(char * const *envp);

/* Builds an environment block, as __libc16_envblock does. */
typedef unsigned (*__libc16_envfn)(char * const *envp);

/* INT 21h 49h: frees the block at seg, from __libc16_envblock. */
void __libc16_dosfree(unsigned seg);
#pragma aux __libc16_dosfree = \
    "mov ah, 49h" \
    "int 21h" \
    parm [es] modify exact [ax];

/* system and libc16_systeme: runs COMSPEC (or \COMMAND.COM) with the
 * tail " /C " and cmd as it is, and, if env and envp are not null, the
 * environment block env builds from envp; for a null cmd, 1 if the
 * command processor exists, else 0 (crt/system.c). system passes a null
 * env, so links no environment block. */
int __libc16_system(const char *cmd, char * const *envp, __libc16_envfn env);

/* Writes every stream's pending output before a child runs
 * (crt/stdio16.h); if that fails, frees the environment block seg (if
 * not 0) and is nonzero, with errno from the failed write, and the
 * caller returns -1 without running the child. A macro, so that a
 * program without buffering links nothing more than bufhook. */
#define FLUSH_FAILED(seg) \
    (BUFCTL(NULL, BUF_ALL) != 0 && ((seg) == 0 || (__libc16_dosfree(seg), 1)))

/* What __libc16_dosexec returns for DOS errors 2, 3 and 15: try the next
 * name. */
#define NOT_FOUND (-2)

#endif
