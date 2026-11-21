/* libc16_globfree - libc16 (docs/spec/glob.md) */

#include <libc16.h>
#include <stdlib.h>

void __cdecl libc16_globfree(libc16_glob_t *g)
{
    while (g->gl_pathc != 0)
        free(g->gl_pathv[--g->gl_pathc]);
    free(g->gl_pathv);
    g->gl_pathv = NULL;
    g->__gl_slots = 0;
}
