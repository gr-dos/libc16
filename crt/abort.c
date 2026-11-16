/* abort - libc16 (docs/spec/stdlib.md) */

#include <stdlib.h>
#include "io16.h"

static const char msg[] = "abnormal program termination\r\n";

/* INT 21h 40h to handle 2; an error is ignored */
static void write_err(const char *s, unsigned n);
#pragma aux write_err = \
    "mov bx, 2" \
    "mov ah, 40h" \
    "int 21h" \
    parm DS_DX [cx] modify exact [ax bx];

/* INT 21h 4Ch with exit code 3 */
static void terminate(void);
#pragma aux terminate = \
    "mov ax, 4c03h" \
    "int 21h" \
    aborts;

void __cdecl abort(void)
{
    write_err(msg, sizeof msg - 1);
    terminate();
}
