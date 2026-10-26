/* A __cdecl callback parameter accepts a __cdecl function. */

typedef int (__cdecl *cmp_t)(const void *, const void *);
extern void __cdecl sortit(cmp_t f);

static int __cdecl cmp(const void *a, const void *b)
{
    return a != b;
}

void sort(void)
{
    sortit(cmp);
}
