/* Uses only the 64-bit signed divide, so links only it and the unsigned
 * core it shares (tests/arith/members). Exit status 0 if the quotient is
 * right. */

long long a = -1234567890123456789LL, b = 987654321LL;

int main(void)
{
    return a / b == -1249999988LL ? 0 : 1;
}
