#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <string>

#include "sf_arithmetic.h"

struct FreeBig {
    void operator()(sfbigint_t* p) const { sfbigint_free(p); }
};
using Big = std::unique_ptr<sfbigint_t, FreeBig>;
static Big number(const char* s) {
    sfbigint_t* p = nullptr;
    EXPECT_EQ(sfbigint_strtosfbigint(&p, s, strlen(s)), SF_ARITHMETIC_FINE);
    return Big(p);
}
static void equals(sfbigint_t* p, const char* expected) {
    char* s = nullptr;
    ASSERT_EQ(sfbigint_tostring(p, &s), SF_ARITHMETIC_FINE);
    EXPECT_STREQ(s, expected);
    EXPECT_EQ(strlen(s), strlen(expected));
    SF_ARITHMETIC_FREE(s);
    ASSERT_GE(p->size, 1u);
    if (p->size > 1) {
        EXPECT_NE(p->digits[p->size - 1], 0u);
    }
    if (sfbigint_iszero(p) == SF_ARITHMETIC_TRUE) {
        EXPECT_EQ(p->sign, SF_ARITHMETIC_PLUS);
    }
}

TEST(regression, decimal_boundaries) {
    for (const char* s :
         {"0", "1", "-1", "4294967295", "4294967296", "4294967297",
          "10000000000", "12884901888", "18446744073709551616",
          "-12345678901234567890123456789012345678901234567890"}) {
        auto p = number(s);
        ASSERT_TRUE(p);
        equals(p.get(), s);
    }
    auto p = number("+000123");
    equals(p.get(), "123");
    p = number("-0000");
    equals(p.get(), "0");
    p = number("8589934591");
    ASSERT_EQ(sfbigint_mul10(p.get()), 0);
    equals(p.get(), "85899345910");
}
TEST(regression, invalid_input_and_nulls) {
    sfbigint_t* p = nullptr;
    for (const char* s : {"", "+", "-", "1a", " 1", "1 ", "--1", "1.5"}) {
        EXPECT_EQ(sfbigint_strtosfbigint(&p, s, strlen(s)),
                  SF_ARITHMETIC_INVALID_ARGUMENT);
        EXPECT_EQ(p, nullptr);
    }
    EXPECT_EQ(sfbigint_strtosfbigint(&p, nullptr, 0),
              SF_ARITHMETIC_INVALID_ARGUMENT);
    EXPECT_EQ(sfbigint_create(nullptr, 1), SF_ARITHMETIC_INVALID_ARGUMENT);
    EXPECT_EQ(sfbigint_create(&p, 0), SF_ARITHMETIC_INVALID_ARGUMENT);
    EXPECT_EQ(sfbigint_free(nullptr), SF_ARITHMETIC_FINE);
    auto a = number("12"), b = number("0"), r = number("123");
    EXPECT_EQ(sfbigint_div(a.get(), b.get(), r.get()),
              SF_ARITHMETIC_INVALID_ARGUMENT);
    equals(r.get(), "123");
    EXPECT_EQ(sfbigint_add(nullptr, a.get(), r.get()),
              SF_ARITHMETIC_MEMORY_ERROR);
    EXPECT_EQ(sfbigint_copy(a.get(), nullptr), SF_ARITHMETIC_INVALID_ARGUMENT);
    EXPECT_EQ(sfbigint_move(a.get(), nullptr), SF_ARITHMETIC_INVALID_ARGUMENT);
    EXPECT_EQ(sfbigint_tostring(a.get(), nullptr),
              SF_ARITHMETIC_INVALID_ARGUMENT);
    EXPECT_EQ(sfbigint_divmod10(a.get(), nullptr),
              SF_ARITHMETIC_INVALID_ARGUMENT);
    EXPECT_EQ(sfbigint_mul_karatsuba(a.get(), b.get(), nullptr),
              SF_ARITHMETIC_INVALID_ARGUMENT);
    equals(a.get(), "12");
}
TEST(regression, subtraction_borrows_and_inputs) {
    auto a = number("18446744073709551616"), b = number("1"), r = number("-9");
    ASSERT_EQ(sfbigint_sub(a.get(), b.get(), r.get()), 0);
    equals(r.get(), "18446744073709551615");
    equals(a.get(), "18446744073709551616");
    equals(b.get(), "1");
    ASSERT_EQ(sfbigint_sub(b.get(), a.get(), r.get()), 0);
    equals(r.get(), "-18446744073709551615");
    equals(a.get(), "18446744073709551616");
    equals(b.get(), "1");
}
TEST(regression, sign_and_result_reuse) {
    auto a = number("-3"), b = number("-2"), r = number("0");
    ASSERT_EQ(sfbigint_add(a.get(), b.get(), r.get()), 0);
    equals(r.get(), "-5");
    equals(a.get(), "-3");
    equals(b.get(), "-2");
    a = number("1");
    b = number("2");
    ASSERT_EQ(sfbigint_add(a.get(), b.get(), r.get()), 0);
    equals(r.get(), "3");
    ASSERT_EQ(sfbigint_sub(a.get(), a.get(), r.get()), 0);
    equals(r.get(), "0");
    a = number("-5");
    b = number("3");
    EXPECT_EQ(sfbigint_compare(a.get(), b.get()), 2);
    b = number("-6");
    EXPECT_EQ(sfbigint_compare(a.get(), b.get()), 1);
}
TEST(regression, shifts_and_zero) {
    auto a = number("4294967296");
    ASSERT_EQ(sfbigint_rshift(a.get(), 32), 0);
    equals(a.get(), "1");
    ASSERT_EQ(sfbigint_rshift(a.get(), 32), 0);
    equals(a.get(), "0");
    a = number("-18446744073709551616");
    ASSERT_EQ(sfbigint_rshift(a.get(), std::numeric_limits<size_t>::max()), 0);
    equals(a.get(), "0");
    a = number("18446744073709551616");
    ASSERT_EQ(sfbigint_setzero(a.get()), 0);
    equals(a.get(), "0");
    a = number("-3");
    ASSERT_EQ(sfbigint_rshift(a.get(), 1), 0);
    equals(a.get(), "-1");
    ASSERT_EQ(sfbigint_lshift(a.get(), 65), 0);
    equals(a.get(), "-36893488147419103232");
}
TEST(regression, multiplication_and_division_aliases) {
    auto a = number("2147483648"), b = number("2"), r = number("-1");
    ASSERT_EQ(sfbigint_mul(a.get(), b.get(), r.get()), 0);
    equals(r.get(), "4294967296");
    ASSERT_EQ(sfbigint_mul(a.get(), b.get(), a.get()), 0);
    equals(a.get(), "4294967296");
    ASSERT_EQ(sfbigint_div(a.get(), b.get(), b.get()), 0);
    equals(b.get(), "2147483648");
    ASSERT_EQ(sfbigint_add(a.get(), b.get(), b.get()), 0);
    equals(b.get(), "6442450944");
    ASSERT_EQ(sfbigint_subeq(b.get(), a.get()), 0);
    equals(b.get(), "2147483648");
    ASSERT_EQ(sfbigint_addeq(a.get(), a.get()), 0);
    equals(a.get(), "8589934592");
    ASSERT_EQ(sfbigint_div(a.get(), a.get(), a.get()), 0);
    equals(a.get(), "1");
    a = number("-7");
    b = number("3");
    ASSERT_EQ(sfbigint_div(a.get(), b.get(), r.get()), 0);
    equals(r.get(), "-2");
}
TEST(regression, copy_move_and_padded_inputs) {
    auto a = number("-123"), b = number("5");
    ASSERT_EQ(sfbigint_copytoexistent(a.get(), b.get()), 0);
    equals(b.get(), "-123");
    ASSERT_EQ(sfbigint_copytoexistent(a.get(), a.get()), 0);
    equals(a.get(), "-123");
    ASSERT_EQ(sfbigint_movetoexistent(b.get(), b.get()), 0);
    equals(b.get(), "-123");
    sfbigint_t* moved = nullptr;
    auto* old = a.get();
    ASSERT_EQ(sfbigint_move(old, &moved), 0);
    a.release();
    a.reset(moved);
    equals(a.get(), "-123");
    old = a.get();
    ASSERT_EQ(sfbigint_movetoexistent(old, b.get()), 0);
    a.release();
    equals(b.get(), "-123");
    sfbigint_t* p = nullptr;
    ASSERT_EQ(sfbigint_create(&p, 5), 0);
    a.reset(p);
    a->sign = SF_ARITHMETIC_MINUS;
    auto z = number("0");
    EXPECT_EQ(sfbigint_compare(a.get(), z.get()), 0);
    EXPECT_EQ(a->size, 5u);  // comparison must not mutate the operand
    ASSERT_EQ(sfbigint_normalise(a.get()), 0);
    equals(a.get(), "0");
}

TEST(regression, all_operands_alias) {
    auto a = number("18446744073709551615");
    ASSERT_EQ(sfbigint_mul(a.get(), a.get(), a.get()), 0);
    equals(a.get(), "340282366920938463426481119284349108225");
    ASSERT_EQ(sfbigint_sub(a.get(), a.get(), a.get()), 0);
    equals(a.get(), "0");
}

TEST(fast_arithmetic, knuth_addback_and_saturated_estimate) {
    {
        auto a = number("170141183460469231731687303720179073023"),
             b = number("39614081257132168796771975169"), r = number("-1");
        ASSERT_EQ(sfbigint_div(a.get(), b.get(), r.get()), 0);
        equals(r.get(), "4294967295");
        equals(a.get(), "170141183460469231731687303720179073023");
        equals(b.get(), "39614081257132168796771975169");
    }
    {
        auto a = number("170141183500083312988819472516951048192"),
             b = number("39614081257132168796771975169"), r = number("-1");
        ASSERT_EQ(sfbigint_div(a.get(), b.get(), r.get()), 0);
        equals(r.get(), "4294967296");
        equals(a.get(), "170141183500083312988819472516951048192");
        equals(b.get(), "39614081257132168796771975169");
    }
}

TEST(fast_arithmetic, fft_sign_carries_and_null_output) {
    auto a = number("-18446744073709551615"),
         b = number("18446744073709551615");
    sfbigint_t* out = nullptr;
    ASSERT_EQ(sfbigint_mul_fft(a.get(), b.get(), &out), 0);
    Big result(out);
    equals(result.get(), "-340282366920938463426481119284349108225");
    equals(a.get(), "-18446744073709551615");
    EXPECT_EQ(sfbigint_mul_fft(a.get(), b.get(), nullptr),
              SF_ARITHMETIC_INVALID_ARGUMENT);
    out = nullptr;
    EXPECT_EQ(sfbigint_mul_fft(nullptr, b.get(), &out),
              SF_ARITHMETIC_MEMORY_ERROR);
    EXPECT_EQ(out, nullptr);
    a = number("0");
    ASSERT_EQ(sfbigint_mul_fft(a.get(), b.get(), &out), 0);
    result.reset(out);
    equals(result.get(), "0");
}

TEST(fast_arithmetic, padded_inputs_and_large_square_alias) {
    sfbigint_t *pa = nullptr, *pb = nullptr;
    ASSERT_EQ(sfbigint_create(&pa, 5), 0);
    Big a(pa);
    ASSERT_EQ(sfbigint_create(&pb, 4), 0);
    Big b(pb);
    a->digits[2] = 1;
    a->digits[0] = 7;
    b->digits[1] = 1;
    b->digits[0] = 1;
    auto r = number("-12");
    ASSERT_EQ(sfbigint_div(a.get(), b.get(), r.get()), 0);
    equals(r.get(), "4294967295");
    EXPECT_EQ(a->size, 5u);
    EXPECT_EQ(b->size, 4u);
    pa = nullptr;
    constexpr size_t limbs = SF_ARITHMETIC_FFT_THRESHOLD;
    ASSERT_EQ(sfbigint_create(&pa, static_cast<uint32_t>(limbs)), 0);
    a.reset(pa);
    for (size_t i = 0; i < limbs; ++i)
        a->digits[i] = UINT32_MAX;
    ASSERT_EQ(sfbigint_mul(a.get(), a.get(), a.get()), 0);
    ASSERT_EQ(a->size, 2 * limbs);
    // (B^n - 1)^2 = B^(2n) - 2*B^n + 1.
    EXPECT_EQ(a->digits[0], 1u);
    for (size_t i = 1; i < limbs; ++i)
        EXPECT_EQ(a->digits[i], 0u);
    EXPECT_EQ(a->digits[limbs], UINT32_MAX - 1);
    for (size_t i = limbs + 1; i < 2 * limbs; ++i)
        EXPECT_EQ(a->digits[i], UINT32_MAX);
}

TEST(fast_arithmetic, division_fast_paths) {
    auto a = number("-340282366920938463463374607431768211473");
    auto b = number("36893488147419103232"), r = number("99");
    ASSERT_EQ(sfbigint_div(a.get(), b.get(), r.get()), 0);
    equals(r.get(), "-9223372036854775808");
    b = number("-1");
    ASSERT_EQ(sfbigint_div(a.get(), b.get(), r.get()), 0);
    equals(r.get(), "340282366920938463463374607431768211473");
    b = number("3");
    ASSERT_EQ(sfbigint_div(a.get(), b.get(), r.get()), 0);
    equals(r.get(), "-113427455640312821154458202477256070491");
    ASSERT_EQ(sfbigint_div(b.get(), a.get(), r.get()), 0);
    equals(r.get(), "0");
    ASSERT_EQ(sfbigint_div(a.get(), a.get(), r.get()), 0);
    equals(r.get(), "1");
}

TEST(fast_arithmetic, knuth_two_estimate_corrections) {
    auto a = number("29842292235457605469042568588"),
         b = number("9841814792810557426"), r = number("0");
    ASSERT_EQ(sfbigint_div(a.get(), b.get(), r.get()), 0);
    equals(r.get(), "3032194047");
}
