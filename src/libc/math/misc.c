/* absolutes */

int abs(int n) {
    return n < 0 ? -n : n;
}

long labs(long n) {
    return n < 0l ? -n : n;
}

long long llabs(long long n) {
    return n < 0ll ? -n : n;
}

float fabsf(float n) {
    return n < 0.0f ? -n : n;
}

double fabs(double n) {
    return n < 0.0 ? -n : n;
}

long double fabsl(long double n) {
    return n < 0.0l ? -n : n;
}

/* division */

struct div_t { int quot; int rem; };
struct ldiv_t { long quot; long rem; };
struct lldiv_t { long long quot; long long rem; };

struct div_t div(int x, int y) {
    return (struct div_t) { x / y, x % y };
}

struct ldiv_t ldiv(long x, long y) {
    return (struct ldiv_t) { x / y, x % y };
}

struct lldiv_t lldiv(long long x, long long y) {
    return (struct lldiv_t) { x / y, x % y };
}

// TODO: rest of floating point stuff
