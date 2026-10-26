/* Library variables, declared with and without __cdecl. */

extern unsigned __cdecl cvar;
extern unsigned wvar;

unsigned sum(void)
{
    return cvar + wvar;
}
