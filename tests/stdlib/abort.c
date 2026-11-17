/* abort (docs/spec/stdlib.md): registers an atexit function, which abort
 * must not call, makes handle 2 the file ABORT.ERR and calls abort, which
 * must end the program with exit code 3; abortchk then checks what it
 * wrote. */

#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "testio.h"

/* INT 21h 46h: make handle 2 refer to what fd refers to */
static unsigned dup_to_2(int fd);
#pragma aux dup_to_2 = \
    "mov cx, 2" \
    "mov ah, 46h" \
    "int 21h" \
    "sbb ax, ax" \
    parm [bx] value [ax] modify exact [ax cx];

static void __cdecl handler(void)
{
    t_puts("not ok abort called an atexit function\r\n");
}

int main(void)
{
    int fd = open("ABORT.ERR", O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);

    if (atexit(handler) != 0 || fd < 0 || dup_to_2(fd) != 0 || close(fd) != 0) {
        t_puts("not ok setup\r\n");
        return 1;
    }
    t_puts("aborting\r\n");
    abort();
    t_puts("not ok abort returned\r\n");
    return 1;
}
