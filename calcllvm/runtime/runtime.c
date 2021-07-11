#include "math.h"
#include "stdint.h"
#include "stdio.h"

int64_t powi_(int64_t b, int64_t e) {
    if (e == 1)
        return b;

    if ((e % 2) == 0) {
        auto r = powi_(b, e / 2);
        return r * r;
    } else {
        auto r = powi_(b, e / 2);
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

    return powi_(b, e);
}

void outputi(int64_t v) {
    printf("%lld\n", v);
}

void outputf(double v) {
    printf("%lf\n", v);
}
