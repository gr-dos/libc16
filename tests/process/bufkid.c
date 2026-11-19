/* Buffered output and child programs (docs/spec/process.md, "Across the
 * call"; docs/spec/stdio.md, "Buffering"): system, libc16_systeme and the
 * spawn functions write the streams' pending output before the child
 * runs, so the child's output, on the same handle, comes after it. Run
 * with COMSPEC=C:\CHILD.EXE; bufkid.expect checks the order. */

#include <process.h>
#include <stdio.h>
#include <stdlib.h>

static char lbuf[64];

int main(void)
{
    if (setvbuf(stdout, NULL, _IOFBF, 0) != 0)
        return 1;
    fputs("before spawnl\n", stdout);
    if (spawnl(P_WAIT, "CHILD.EXE", "CHILD.EXE", "spawned", NULL) != 0)
        return 2;
    fputs("before system\n", stdout);
    if (system("run") != 0)
        return 3;
    fputs("before libc16_systeme\n", stdout);
    if (libc16_systeme("again", NULL) != 0)
        return 4;
    /* a line-buffered stream's partial line too */
    if (setvbuf(stdout, lbuf, _IOLBF, sizeof lbuf) != 0)
        return 5;
    fputs("partial: ", stdout);
    if (spawnl(P_WAIT, "CHILD.EXE", "CHILD.EXE", "last", NULL) != 0)
        return 6;
    fputs("end\n", stdout);
    return 0;
}
