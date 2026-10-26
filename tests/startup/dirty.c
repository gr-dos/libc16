/* Fills BSS with 0AAh before exiting, so that bss.exe, run next, is
 * loaded into memory that is not already zero. */

unsigned char junk[16384];

int main(void)
{
    unsigned i;

    for (i = 0; i < sizeof junk; i++)
        junk[i] = 0xAA;
    return 0;
}
