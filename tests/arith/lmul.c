/* Uses only the 32-bit multiply, so links only its member
 * (tests/arith/members). Exit status 0 if the product is right. */

long a = 123456L, b = -7890L;

int main(void)
{
    return a * b == -974067840L ? 0 : 1;
}
