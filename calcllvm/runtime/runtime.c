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

struct Value {
    // -1, if unintialized. 0 for int, 1 for float.
    char type;
    union {
        int64_t i;
        double f;
    } v;
};

extern int num_values;
extern char* names[128];
static struct Value values[128];

void set(int i, char type, int64_t bytes) {
    values[i].type = type;
    values[i].v.i = bytes;
}

int64_t get_int(int i) {
    return values[i].v.i;
}

double get_fp(int i) {
    return values[i].v.f;
}
