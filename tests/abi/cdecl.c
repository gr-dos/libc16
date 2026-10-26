/* A call to a __cdecl library function from default-convention code. */

extern unsigned __cdecl libfn(const char *s, int n);

unsigned call(const char *s, int n)
{
    return libfn(s, n);
}
