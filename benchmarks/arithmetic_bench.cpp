#include <algorithm>
#include <chrono>
#include <cstdio>
#include <memory>
#include <random>
#include <stdexcept>

#include "sf_arithmetic.h"

struct FreeBig {
    void operator()(sfbigint_t* p) const { sfbigint_free(p); }
};
using Big = std::unique_ptr<sfbigint_t, FreeBig>;
static Big number(size_t limbs, std::mt19937& random) {
    sfbigint_t* p = nullptr;
    if (sfbigint_create(&p, static_cast<uint32_t>(limbs)))
        throw std::bad_alloc();
    Big result(p);
    for (size_t i = 0; i < limbs; ++i)
        p->digits[i] = random();
    p->digits[limbs - 1] |= UINT32_C(0x80000000);
    return result;
}
using Multiply = int (*)(sfbigint_t*, sfbigint_t*, sfbigint_t**);
static double multiply(Multiply method, sfbigint_t* a, sfbigint_t* b,
                       size_t rounds) {
    auto start = std::chrono::steady_clock::now();
    for (size_t i = 0; i < rounds; ++i) {
        sfbigint_t* out = nullptr;
        int rc = method(a, b, &out);
        Big result(out);
        if (rc)
            throw std::runtime_error("multiplication failed");
    }
    return std::chrono::duration<double, std::micro>(
               std::chrono::steady_clock::now() - start)
               .count() /
           rounds;
}
int main() {
    try {
        std::mt19937 random(20260920);
        std::puts("Multiplication: limbs, Karatsuba us, FFT/NTT us");
        for (size_t n : {32u, 128u, 256u, 512u, 1024u, 2048u, 4096u, 8192u}) {
            auto a = number(n, random), b = number(n, random);
            size_t rounds = std::max(size_t(1), size_t(8192) / n);
            double karatsuba =
                multiply(sfbigint_mul_karatsuba, a.get(), b.get(), rounds);
#ifdef SF_ARITHMETIC_FFT_THRESHOLD
            double fft = multiply(sfbigint_mul_fft, a.get(), b.get(), rounds);
            sfbigint_t *x = nullptr, *y = nullptr;
            int rx = sfbigint_mul_karatsuba(a.get(), b.get(), &x);
            Big bx(x);
            int ry = sfbigint_mul_fft(a.get(), b.get(), &y);
            Big by(y);
            if (rx || ry || sfbigint_compare(x, y))
                throw std::runtime_error("product mismatch");
            std::printf("%zu, %.2f, %.2f\n", n, karatsuba, fft);
#else
            std::printf("%zu, %.2f, unavailable\n", n, karatsuba);
#endif
        }
        std::puts("Division (2N/N): divisor limbs, us");
        for (size_t n : {1u, 8u, 32u, 128u, 512u, 1024u}) {
            auto a = number(2 * n, random), b = number(n, random),
                 out = number(1, random);
            size_t rounds = std::max(size_t(1), size_t(1024) / (n * n));
            auto start = std::chrono::steady_clock::now();
            for (size_t i = 0; i < rounds; ++i)
                if (sfbigint_div(a.get(), b.get(), out.get()))
                    throw std::runtime_error("division failed");
            double us = std::chrono::duration<double, std::micro>(
                            std::chrono::steady_clock::now() - start)
                            .count() /
                        rounds;
            std::printf("%zu, %.2f\n", n, us);
        }
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
    return 0;
}
