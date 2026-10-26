/* ferror - libc16 (docs/spec/stdio.md); <stdio.h> also has a macro */

#include <stdio.h>

#undef ferror

int __cdecl ferror(FILE *fp)
{
    return (fp->_flag & _IOERR) != 0;
}
