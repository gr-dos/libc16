/* spawnl, spawnv and spawnve without the PATH search link no getenv, heap
 * or stdio (docs/spec/process.md); tests/process/members checks it. */

#include <errno.h>
#include <process.h>
#include "check.h"

int main(void)
{
    static char *argv[] = { "child", "6", 0 };
    static char *envp[] = { "N=1", 0 };

    CHECK(spawnl(P_WAIT, "CHILD.EXE", "child", "2", (char *)0) == 2);
    CHECK(spawnv(P_WAIT, "CHILD", argv) == 6);
    CHECK(spawnve(P_WAIT, "CHILD.EXE", argv, envp) == 6);
    errno = 0;
    CHECK(spawnv(P_WAIT, "NOSUCH", argv) == -1 && errno == ENOENT);
    return done();
}
