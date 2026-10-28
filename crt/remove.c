/* remove - libc16 (docs/spec/files.md) */

#include <io.h>
#include <stdio.h>

int __cdecl remove(const char *path)
{
    return unlink(path);
}
