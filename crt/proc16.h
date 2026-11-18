/* Private to libc16: the spawn functions' core (docs/spec/process.md). */

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

/* What __libc16_dosexec returns for DOS errors 2, 3 and 15: try the next
 * name. */
#define NOT_FOUND (-2)

#endif
