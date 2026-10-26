/* 32- and 64-bit arithmetic the compiler does not inline: one function
 * per helper symbol of docs/spec/arith.md. */

long lmul(long a, long b) { return a * b; }
unsigned long ulmul(unsigned long a, unsigned long b) { return a * b; }
long ldiv(long a, long b) { return a / b; }
unsigned long uldiv(unsigned long a, unsigned long b) { return a / b; }
long long llmul(long long a, long long b) { return a * b; }
unsigned long long ullmul(unsigned long long a, unsigned long long b) { return a * b; }
long long lldiv(long long a, long long b) { return a / b; }
long long llmod(long long a, long long b) { return a % b; }
unsigned long long ulldiv(unsigned long long a, unsigned long long b) { return a / b; }
unsigned long long ullmod(unsigned long long a, unsigned long long b) { return a % b; }
long long llshl(long long a, int n) { return a << n; }
unsigned long long ullshl(unsigned long long a, int n) { return a << n; }
long long llshr(long long a, int n) { return a >> n; }
unsigned long long ullshr(unsigned long long a, int n) { return a >> n; }
