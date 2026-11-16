/* clearerr - libc16 (docs/spec/stdio.md); <stdio.h> also has a macro */

#include <stdio.h>

#undef clearerr

void __cdecl clearerr(FILE *fp)
{
    fp->_flag &= ~(_IOEOF | _IOERR);
}
