/* getenv - libc16 (docs/spec/dos.md) */

#include <dos.h>
#include <stdlib.h>

#define VALUE_MAX 256

static char value[VALUE_MAX];

char * __cdecl getenv(const char *name)
{
    unsigned seg = *(((__segment)_psp) :> ((const unsigned __near *)0x2c));
    const char __far *p = ((__segment)seg) :> ((const char __near *)0);
    const char *n;
    unsigned i;

    if (seg == 0)
        return NULL;
    /* NAME=value strings, each null-terminated, then an empty one */
    while (*p != '\0') {
        for (n = name; *n != '\0' && *n == *p; n++, p++)
            ;
        if (*n == '\0' && *p == '=') {
            p++;
            for (i = 0; i < VALUE_MAX; i++)
                if ((value[i] = p[i]) == '\0')
                    return value;
            return NULL;
        }
        while (*p++ != '\0')
            ;
    }
    return NULL;
}
