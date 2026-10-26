/* Exits 0 if its BSS is zero at main, 1 if not. Run after dirty.exe. */

unsigned char zero[16384];

int main(void)
{
    unsigned i;

    for (i = 0; i < sizeof zero; i++)
        if (zero[i])
            return 1;
    return 0;
}
