/* calloc - libc16 (docs/spec/heap.md; C99 7.20.3.1)
 *
 * malloc of n * size bytes, zeroed. A product that does not fit in a
 * size_t fails with ENOMEM, as malloc fails for a size too large. */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

void * __cdecl calloc(size_t n, size_t size)
{
    void *p;

    if (size && n > (size_t)-1 / size) {
        errno = ENOMEM;
        return NULL;
    }
    p = malloc(n * size);
    if (p)
        memset(p, 0, n * size);
    return p;
}
