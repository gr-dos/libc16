/* A default-convention function passed as a __cdecl callback: must not
 * compile. */

typedef int (__cdecl *cmp_t)(const void *, const void *);
extern void __cdecl sortit(cmp_t f);

static int cmp(const void *a, const void *b)
{
    return a != b;
}

void sort(void)
{
    sortit(cmp);
}
