/* Exits 0 if a pointer to its first string literal is not a null
 * pointer, 1 if it is (docs/spec/startup.md, "Null pointer"). It is
 * linked first, then tests/startup/codepad.asm, which ends the code at a
 * paragraph, so that without the startup's reserved bytes this module's
 * CONST, and the literal, would be at offset 0 of DGROUP. */

const char *first = "first";

int main(void)
{
    return first != 0 && first[0] == 'f' ? 0 : 1;
}
