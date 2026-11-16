/* The reserved start of DGROUP is at DS:0000 (docs/spec/startup.md,
 * "Null pointer"). Linked first, then tests/startup/codeodd.asm, which
 * ends the code one byte past a paragraph. Exits 0 if the word at
 * DS:0000 is the startup's reserved 0 and a pointer to the first string
 * literal is not a null pointer; 1 if it is; 2 if DS:0000 holds anything
 * else, such as the 90h bytes at the end of the code. */

/* The word at DS:0000, read without a null pointer in C */
static unsigned ds_word0(void);
#pragma aux ds_word0 = \
    "xor bx, bx" \
    "mov ax, [bx]" \
    value [ax] modify exact [ax bx];

const char *first = "first";

int main(void)
{
    if (ds_word0() != 0)
        return 2;
    return first != 0 && first[0] == 'f' ? 0 : 1;
}
