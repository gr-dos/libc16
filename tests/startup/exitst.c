/* exit from a nested call: the exit status is the low byte of its
 * argument, main does not return, and only the core startup and exit are
 * linked. */

#include <stdlib.h>

static void leave(int status)
{
    exit(status);
}

int main(void)
{
    leave(0x1205);
    return 1;
}
