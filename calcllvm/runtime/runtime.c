#include "math.h"
#include "stdint.h"

extern int printf(const char*, ...);

int64_t _powi(int64_t b, int64_t e) {
    if (e == 1)
        return b;

    if ((e % 2) == 0) {
        int64_t r = _powi(b, e / 2);
        return r * r;
    } else {
        int64_t r = _powi(b, e / 2);
        return r * r * b;
    }
}

int64_t powi(int64_t b, int64_t e) {
    if (b == 0 && e == 0)
        return -1;

    if (e < 0)
        return -1;

    if (b == 0)
        return 1;

    return _powi(b, e);
}

void print_i(int64_t v) {
    printf("%lld\n", v);
}

void print_f(double v) {
    printf("%lf\n", v);
}
