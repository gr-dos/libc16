/* fgets - libc16 (docs/spec/stdio.md) */

#include <stdio.h>

char * __cdecl fgets(char *s, int n, FILE *fp)
{
    char *p = s;
    int c;

    if (n <= 0)
        return NULL;
    while (n-- > 1) {
        if ((c = fgetc(fp)) == EOF) {
            /* fgetc sets the end-of-file indicator, or else the error one */
            if (p == s || !feof(fp))
                return NULL;
            break;
        }
        *p++ = (char)c;
        if (c == '\n')
            break;
    }
    *p = '\0';
    return s;
}
