/* feof - libc16 (docs/spec/stdio.md); <stdio.h> also has a macro */

#include <stdio.h>

#undef feof

int __cdecl feof(FILE *fp)
{
    return (fp->_flag & _IOEOF) != 0;
}
