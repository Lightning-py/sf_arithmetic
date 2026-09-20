#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t live_allocations, calls;
static long fail_at = -1;
static void* test_malloc(size_t size) {
    if ((long)calls++ == fail_at)
        return NULL;
    void* p = malloc(size);
    if (p)
        ++live_allocations;
    return p;
}
static void* test_calloc(size_t n, size_t size) {
    if ((long)calls++ == fail_at)
        return NULL;
    void* p = calloc(n, size);
    if (p)
        ++live_allocations;
    return p;
}
static void test_free(void* p) {
    if (p) {
        --live_allocations;
        free(p);
    }
}
#define SF_ARITHMETIC_MALLOC test_malloc
#define SF_ARITHMETIC_CALLOC test_calloc
#define SF_ARITHMETIC_FREE test_free
#include "../src/sf_arithmetic.c"

#define CHECK(expr)                                                            \
    do {                                                                       \
        if (!(expr)) {                                                         \
            fprintf(stderr,                                                    \
                    "check failed: %s at line %d (op=%d, failure=%ld)\n",      \
                    #expr, __LINE__, op, index);                               \
            return 1;                                                          \
        }                                                                      \
    } while (0)

int main(void) {
    size_t checked = 0;
    for (int op = 0; op < 22; ++op) {
        int completed = 0;
        for (long index = 0; index < 4096; ++index) {
            fail_at = -1;
            sfbigint_t *a = NULL, *b = NULL, *r = NULL, *out = NULL;
            char* str = NULL;
            CHECK(!sfbigint_create(&a, 7));
            CHECK(!sfbigint_create(&b, 6));
            CHECK(!sfbigint_create(&r, 2));
            for (size_t i = 0; i < a->size; ++i)
                a->digits[i] = UINT32_MAX;
            for (size_t i = 0; i < b->size; ++i)
                b->digits[i] = UINT32_MAX - 1;
            a->sign = SF_ARITHMETIC_MINUS;
            r->digits[0] = 123;
            r->digits[1] = 456;
            uint32_t ad[7], bd[6], rd[2];
            memcpy(ad, a->digits, sizeof ad);
            memcpy(bd, b->digits, sizeof bd);
            memcpy(rd, r->digits, sizeof rd);
            uint32_t *ap = a->digits, *bp = b->digits, *rp = r->digits;
            calls = 0;
            fail_at = index;
            int rc = 0;
            switch (op) {
                case 0:
                    rc = sfbigint_create(&out, 3);
                    break;
                case 1:
                    rc = sfbigint_copy(a, &out);
                    break;
                case 2:
                    rc = sfbigint_move(a, &out);
                    if (!rc)
                        a = NULL;
                    break;
                case 3:
                    rc = sfbigint_copytoexistent(a, r);
                    break;
                case 4:
                    rc = sfbigint_movetoexistent(a, r);
                    if (!rc)
                        a = NULL;
                    break;
                case 5:
                    rc = sfbigint_add(a, b, r);
                    break;
                case 6:
                    rc = sfbigint_sub(a, b, r);
                    break;
                case 7:
                    rc = sfbigint_mul(a, b, r);
                    break;
                case 8:
                    rc = sfbigint_div(a, b, r);
                    break;
                case 9:
                    rc = sfbigint_lshift(a, 65);
                    break;
                case 10:
                    rc = sfbigint_mul10(a);
                    break;
                case 11: {
                    const char* s = "-12345678901234567890123456789012345678901"
                                    "2345678901234567890";
                    rc = sfbigint_strtosfbigint(&out, s, strlen(s));
                    break;
                }
                case 12:
                    rc = sfbigint_tostring(a, &str);
                    break;
                case 13:
                    rc = sfbigint_mul_karatsuba(a, b, &out);
                    break;
                case 14:
                    rc = sfbigint_addeq(a, b);
                    break;
                case 15:
                    rc = sfbigint_subeq(a, b);
                    break;
                case 16:
                    rc = sfbigint_mul(a, b, a);
                    break;
                case 17:
                    rc = sfbigint_div(a, b, b);
                    break;
                case 18:
                    rc = sfbigint_add(a, b, b);
                    break;
                case 19:
                    rc = __sfbigint_add__(a, b, r);
                    break;
                case 21:
                    rc = sfbigint_mul_fft(a, b, &out);
                    break;
                case 20:
                    rc = __sfbigint_sub__(b, a, r);
                    break;
            }
            fail_at = -1;
            CHECK(rc == SF_ARITHMETIC_FINE || rc == SF_ARITHMETIC_MEMORY_ERROR);
            if (rc) {
                CHECK(a && a->digits == ap && a->size == 7 &&
                      a->sign == SF_ARITHMETIC_MINUS);
                CHECK(b->digits == bp && b->size == 6 &&
                      b->sign == SF_ARITHMETIC_PLUS);
                CHECK(r->digits == rp && r->size == 2 &&
                      r->sign == SF_ARITHMETIC_PLUS);
                CHECK(!memcmp(a->digits, ad, sizeof ad));
                CHECK(!memcmp(b->digits, bd, sizeof bd));
                CHECK(!memcmp(r->digits, rd, sizeof rd));
                CHECK(!out && !str);
            }
            sfbigint_free(a);
            sfbigint_free(b);
            sfbigint_free(r);
            sfbigint_free(out);
            test_free(str);
            CHECK(live_allocations == 0);
            ++checked;
            if (!rc) {
                completed = 1;
                break;
            }
        }
        long index = -1;
        CHECK(completed);
    }
    printf("%zu allocation-failure/success scenarios passed.\n", checked);
    return 0;
}
