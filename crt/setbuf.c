/* setbuf - libc16 (docs/spec/stdio.md, "Buffering") */

#include <stdio.h>
#include "stdio16.h"

void __cdecl setbuf(FILE *fp, char *buf)
{
    __libc16_bufset(fp, buf, buf != NULL ? _IOFBF : _IONBF, BUFSIZ, NULL);
}
