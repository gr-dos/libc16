/* 32- and 64-bit arithmetic the compiler does not inline. */

long lmul(long a, long b) { return a * b; }
unsigned long udiv(unsigned long a, unsigned long b) { return a / b; }
long long llmul(long long a, long long b) { return a * b; }
long long lldiv(long long a, long long b) { return a / b; }
