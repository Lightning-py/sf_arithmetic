#include <chrono>
#include <cstdio>
#include <random>

#include "sf_arithmetic.h"

int main() {
    constexpr size_t iterations = 100000;
    sfbigint_t *a = nullptr, *b = nullptr, *r = nullptr;
    if (sfbigint_create(&a, 100) || sfbigint_create(&b, 100) ||
        sfbigint_create(&r, 1)) {
        sfbigint_free(a);
        sfbigint_free(b);
        sfbigint_free(r);
        return 1;
    }
    std::mt19937 rng(42);
    for (size_t i = 0; i < 100; ++i) {
        a->digits[i] = rng();
        b->digits[i] = rng();
    }
    auto start = std::chrono::steady_clock::now();
    int rc = 0;
    for (size_t i = 0; i < iterations; ++i) {
        rc = sfbigint_add(a, b, r);
        if (rc)
            break;
    }
    auto elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start);
    if (!rc)
        std::printf("%zu additions of 100 limbs: %.6f s; checksum=%u\n",
                    iterations, elapsed.count(), r->digits[0]);
    sfbigint_free(a);
    sfbigint_free(b);
    sfbigint_free(r);
    return rc ? 1 : 0;
}
