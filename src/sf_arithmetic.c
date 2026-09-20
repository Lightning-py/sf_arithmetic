#include "sf_arithmetic.h"

/* size is the number of significant limbs, not allocation capacity. */
static size_t used(const sfbigint_t* a) {
    size_t n = a->size;
    while (n > 1 && a->digits[n - 1] == 0)
        --n;
    return n;
}

static int zero(const sfbigint_t* a) {
    return used(a) == 1 && a->digits[0] == 0;
}

static void trim(sfbigint_t* a) {
    a->size = used(a);
    if (zero(a))
        a->sign = SF_ARITHMETIC_PLUS;
}

static int alloc_number(sfbigint_t** out, size_t n) {
    if (!out || !n)
        return SF_ARITHMETIC_INVALID_ARGUMENT;
    if (n > SIZE_MAX / sizeof(uint32_t))
        return SF_ARITHMETIC_MEMORY_ERROR;
    sfbigint_t* a = SF_ARITHMETIC_MALLOC(sizeof(*a));
    if (!a)
        return SF_ARITHMETIC_MEMORY_ERROR;
    a->digits = SF_ARITHMETIC_CALLOC(n, sizeof(*a->digits));
    if (!a->digits) {
        SF_ARITHMETIC_FREE(a);
        return SF_ARITHMETIC_MEMORY_ERROR;
    }
    a->size = n;
    a->sign = SF_ARITHMETIC_PLUS;
    *out = a;
    return SF_ARITHMETIC_FINE;
}

/* Transfer a completed temporary into an existing object; cannot fail. */
static void commit(sfbigint_t* dst, sfbigint_t* tmp) {
    SF_ARITHMETIC_FREE(dst->digits);
    *dst = *tmp;
    SF_ARITHMETIC_FREE(tmp);
}

int sfbigint_fine(sfbigint_t* a) {
    return a && a->digits && a->size &&
                   a->size <= SIZE_MAX / sizeof(*a->digits) &&
                   (a->sign == SF_ARITHMETIC_PLUS ||
                    a->sign == SF_ARITHMETIC_MINUS)
               ? SF_ARITHMETIC_FINE
               : SF_ARITHMETIC_MEMORY_ERROR;
}

int sfbigint_create(sfbigint_t** out, SF_ARITHMETIC_DIGITS_T n) {
    return alloc_number(out, n);
}

int sfbigint_free(sfbigint_t* a) {
    if (!a)
        return SF_ARITHMETIC_FINE;
    SF_ARITHMETIC_FREE(a->digits);
    SF_ARITHMETIC_FREE(a);
    return SF_ARITHMETIC_FINE;
}

int sfbigint_normalise(sfbigint_t* a) {
    if (sfbigint_fine(a))
        return SF_ARITHMETIC_MEMORY_ERROR;
    trim(a);
    return SF_ARITHMETIC_FINE;
}

int sfbigint_setzero(sfbigint_t* a) {
    if (sfbigint_fine(a))
        return SF_ARITHMETIC_MEMORY_ERROR;
    a->digits[0] = 0;
    a->size = 1;
    a->sign = SF_ARITHMETIC_PLUS;
    return SF_ARITHMETIC_FINE;
}

int sfbigint_iszero(sfbigint_t* a) {
    if (sfbigint_fine(a))
        return SF_ARITHMETIC_MEMORY_ERROR;
    return zero(a);
}

int sfbigint_copy(sfbigint_t* a, sfbigint_t** out) {
    if (!out)
        return SF_ARITHMETIC_INVALID_ARGUMENT;
    if (sfbigint_fine(a))
        return SF_ARITHMETIC_MEMORY_ERROR;
    sfbigint_t* tmp = NULL;
    int rc = alloc_number(&tmp, a->size);
    if (rc)
        return rc;
    memcpy(tmp->digits, a->digits, a->size * sizeof(*a->digits));
    tmp->sign = zero(a) ? SF_ARITHMETIC_PLUS : a->sign;
    *out = tmp;
    return SF_ARITHMETIC_FINE;
}

int sfbigint_move(sfbigint_t* a, sfbigint_t** out) {
    if (!out)
        return SF_ARITHMETIC_INVALID_ARGUMENT;
    if (sfbigint_fine(a))
        return SF_ARITHMETIC_MEMORY_ERROR;
    sfbigint_t* tmp = SF_ARITHMETIC_MALLOC(sizeof(*tmp));
    if (!tmp)
        return SF_ARITHMETIC_MEMORY_ERROR;
    *tmp = *a;
    trim(tmp);
    SF_ARITHMETIC_FREE(a);
    *out = tmp;
    return SF_ARITHMETIC_FINE;
}

int sfbigint_copytoexistent(sfbigint_t* a, sfbigint_t* out) {
    if (sfbigint_fine(a) || sfbigint_fine(out))
        return SF_ARITHMETIC_MEMORY_ERROR;
    if (a == out)
        return SF_ARITHMETIC_FINE;
    sfbigint_t* tmp = NULL;
    int rc = sfbigint_copy(a, &tmp);
    if (!rc)
        commit(out, tmp);
    return rc;
}

int sfbigint_movetoexistent(sfbigint_t* a, sfbigint_t* out) {
    if (sfbigint_fine(a) || sfbigint_fine(out))
        return SF_ARITHMETIC_MEMORY_ERROR;
    if (a != out)
        commit(out, a);
    return SF_ARITHMETIC_FINE;
}

int sfbigint_swap(sfbigint_t* a, sfbigint_t* b) {
    if (sfbigint_fine(a) || sfbigint_fine(b))
        return SF_ARITHMETIC_MEMORY_ERROR;
    sfbigint_t tmp = *a;
    *a = *b;
    *b = tmp;
    return SF_ARITHMETIC_FINE;
}

/* Internal comparison is by magnitude; the public comparison includes sign. */
static int cmp_abs(const sfbigint_t* a, const sfbigint_t* b) {
    size_t na = used(a), nb = used(b);
    if (na != nb)
        return na > nb ? 1 : -1;
    for (size_t i = na; i-- > 0;)
        if (a->digits[i] != b->digits[i])
            return a->digits[i] > b->digits[i] ? 1 : -1;
    return 0;
}

int sfbigint_compare(sfbigint_t* a, sfbigint_t* b) {
    if (sfbigint_fine(a) || sfbigint_fine(b))
        return SF_ARITHMETIC_MEMORY_ERROR;
    int sa = !zero(a) && a->sign, sb = !zero(b) && b->sign;
    int cmp = sa != sb ? (sa ? -1 : 1) : cmp_abs(a, b) * (sa ? -1 : 1);
    return cmp == 0 ? 0 : (cmp > 0 ? 1 : 2);
}

static int add_abs(const sfbigint_t* a, const sfbigint_t* b, sfbigint_t** out) {
    size_t na = used(a), nb = used(b), n = na > nb ? na : nb;
    sfbigint_t* r = NULL;
    int rc = alloc_number(&r, n + 1);
    if (rc)
        return rc;
    uint64_t carry = 0;
    for (size_t i = 0; i < n; ++i) {
        uint64_t v = carry + (i < na ? a->digits[i] : 0) +
                     (uint64_t)(i < nb ? b->digits[i] : 0);
        r->digits[i] = (uint32_t)v;
        carry = v >> 32;
    }
    r->digits[n] = (uint32_t)carry;
    trim(r);
    *out = r;
    return SF_ARITHMETIC_FINE;
}

/* Requires |a| >= |b|. Every limb is written, including the borrow chain. */
static int sub_abs(const sfbigint_t* a, const sfbigint_t* b, sfbigint_t** out) {
    size_t na = used(a), nb = used(b);
    sfbigint_t* r = NULL;
    int rc = alloc_number(&r, na);
    if (rc)
        return rc;
    uint64_t borrow = 0;
    for (size_t i = 0; i < na; ++i) {
        uint64_t sub = (i < nb ? b->digits[i] : 0) + borrow;
        uint64_t v = a->digits[i];
        r->digits[i] = (uint32_t)(v - sub);
        borrow = v < sub;
    }
    trim(r);
    *out = r;
    return SF_ARITHMETIC_FINE;
}

static int signed_sum(sfbigint_t* a, sfbigint_t* b, sfbigint_t* out,
                      int subtract, int magnitude_only) {
    if (sfbigint_fine(a) || sfbigint_fine(b) || sfbigint_fine(out))
        return SF_ARITHMETIC_MEMORY_ERROR;
    int sa = magnitude_only ? 0 : a->sign;
    int sb = (magnitude_only ? 0 : b->sign) ^ subtract;
    sfbigint_t* r = NULL;
    int rc;
    if (sa == sb) {
        rc = add_abs(a, b, &r);
        if (!rc)
            r->sign = sa;
    } else {
        int cmp = cmp_abs(a, b);
        rc = cmp >= 0 ? sub_abs(a, b, &r) : sub_abs(b, a, &r);
        if (!rc)
            r->sign = cmp >= 0 ? sa : sb;
    }
    if (rc)
        return rc;
    trim(r);
    commit(out, r);
    return SF_ARITHMETIC_FINE;
}

int __sfbigint_add__(sfbigint_t* a, sfbigint_t* b, sfbigint_t* r) {
    return signed_sum(a, b, r, 0, 1);
}
int __sfbigint_sub__(sfbigint_t* a, sfbigint_t* b, sfbigint_t* r) {
    return signed_sum(a, b, r, 1, 1);
}
int sfbigint_add(sfbigint_t* a, sfbigint_t* b, sfbigint_t* r) {
    return signed_sum(a, b, r, 0, 0);
}
int sfbigint_sub(sfbigint_t* a, sfbigint_t* b, sfbigint_t* r) {
    return signed_sum(a, b, r, 1, 0);
}
int sfbigint_addeq(sfbigint_t* a, sfbigint_t* b) {
    return sfbigint_add(a, b, a);
}
int sfbigint_subeq(sfbigint_t* a, sfbigint_t* b) {
    return sfbigint_sub(a, b, a);
}

int sfbigint_lshift(sfbigint_t* a, size_t bits) {
    if (sfbigint_fine(a))
        return SF_ARITHMETIC_MEMORY_ERROR;
    if (!bits || zero(a)) {
        trim(a);
        return SF_ARITHMETIC_FINE;
    }
    size_t n = used(a), words = bits / 32;
    unsigned part = (unsigned)(bits % 32);
    if (words > SIZE_MAX - n - 1)
        return SF_ARITHMETIC_MEMORY_ERROR;
    sfbigint_t* r = NULL;
    int rc = alloc_number(&r, n + words + (part != 0));
    if (rc)
        return rc;
    uint64_t carry = 0;
    for (size_t i = 0; i < n; ++i) {
        uint64_t v = ((uint64_t)a->digits[i] << part) | carry;
        r->digits[i + words] = (uint32_t)v;
        carry = v >> 32;
    }
    if (part)
        r->digits[n + words] = (uint32_t)carry;
    r->sign = a->sign;
    trim(r);
    commit(a, r);
    return SF_ARITHMETIC_FINE;
}

int sfbigint_rshift(sfbigint_t* a, size_t bits) {
    if (sfbigint_fine(a))
        return SF_ARITHMETIC_MEMORY_ERROR;
    size_t n = used(a), words = bits / 32;
    unsigned part = (unsigned)(bits % 32);
    if (words >= n)
        return sfbigint_setzero(a);
    /* Ascending traversal only reads limbs not yet overwritten. */
    for (size_t i = 0; i < n - words; ++i) {
        uint32_t v = a->digits[i + words] >> part;
        if (part && i + words + 1 < n)
            v |= a->digits[i + words + 1] << (32 - part);
        a->digits[i] = v;
    }
    a->size = n - words;
    trim(a);
    return SF_ARITHMETIC_FINE;
}

int sfbigint_divmod10(sfbigint_t* a, uint32_t* remainder) {
    if (!remainder)
        return SF_ARITHMETIC_INVALID_ARGUMENT;
    if (sfbigint_fine(a))
        return SF_ARITHMETIC_MEMORY_ERROR;
    uint64_t rem = 0;
    for (size_t i = a->size; i-- > 0;) {
        uint64_t v = (rem << 32) | a->digits[i];
        a->digits[i] = (uint32_t)(v / 10);
        rem = v % 10;
    }
    trim(a);
    *remainder = (uint32_t)rem;
    return SF_ARITHMETIC_FINE;
}

/* Also used for parsing: magnitude = magnitude * 10 + digit. */
static int decimal_step(sfbigint_t* a, uint32_t digit) {
    size_t n = used(a);
    sfbigint_t* r = NULL;
    int rc = alloc_number(&r, n + 1);
    if (rc)
        return rc;
    uint64_t carry = digit;
    for (size_t i = 0; i < n; ++i) {
        uint64_t v = (uint64_t)a->digits[i] * 10 + carry;
        r->digits[i] = (uint32_t)v;
        carry = v >> 32;
    }
    r->digits[n] = (uint32_t)carry;
    r->sign = a->sign;
    trim(r);
    commit(a, r);
    return SF_ARITHMETIC_FINE;
}

int sfbigint_mul10(sfbigint_t* a) {
    if (sfbigint_fine(a))
        return SF_ARITHMETIC_MEMORY_ERROR;
    return decimal_step(a, 0);
}

int sfbigint_strtosfbigint(sfbigint_t** out, const char* str, size_t len) {
    if (!out || !str || !len)
        return SF_ARITHMETIC_INVALID_ARGUMENT;
    size_t start = str[0] == '-' || str[0] == '+' ? 1 : 0;
    if (start == len)
        return SF_ARITHMETIC_INVALID_ARGUMENT;
    for (size_t i = start; i < len; ++i)
        if (str[i] < '0' || str[i] > '9')
            return SF_ARITHMETIC_INVALID_ARGUMENT;
    sfbigint_t* a = NULL;
    int rc = alloc_number(&a, 1);
    if (rc)
        return rc;
    for (size_t i = start; i < len; ++i) {
        rc = decimal_step(a, (uint32_t)(str[i] - '0'));
        if (rc) {
            sfbigint_free(a);
            return rc;
        }
    }
    a->sign =
        str[0] == '-' && !zero(a) ? SF_ARITHMETIC_MINUS : SF_ARITHMETIC_PLUS;
    *out = a;
    return SF_ARITHMETIC_FINE;
}

int sfbigint_tostring(sfbigint_t* a, char** out) {
    if (!out)
        return SF_ARITHMETIC_INVALID_ARGUMENT;
    if (sfbigint_fine(a))
        return SF_ARITHMETIC_MEMORY_ERROR;
    size_t n = used(a);
    if (n > (SIZE_MAX - 2) / 10)
        return SF_ARITHMETIC_MEMORY_ERROR;
    /* A 32-bit limb contributes fewer than ten decimal digits. */
    char* s = SF_ARITHMETIC_MALLOC(n * 10 + 2);
    if (!s)
        return SF_ARITHMETIC_MEMORY_ERROR;
    sfbigint_t* tmp = NULL;
    int rc = sfbigint_copy(a, &tmp);
    if (rc) {
        SF_ARITHMETIC_FREE(s);
        return rc;
    }
    size_t count = 0;
    do {
        uint32_t rem;
        rc = sfbigint_divmod10(tmp, &rem);
        if (rc) {
            sfbigint_free(tmp);
            SF_ARITHMETIC_FREE(s);
            return rc;
        }
        s[count++] = (char)('0' + rem);
    } while (!zero(tmp));
    if (a->sign == SF_ARITHMETIC_MINUS && !zero(a))
        s[count++] = '-';
    for (size_t i = 0; i < count / 2; ++i) {
        char c = s[i];
        s[i] = s[count - i - 1];
        s[count - i - 1] = c;
    }
    s[count] = '\0';
    sfbigint_free(tmp);
    *out = s;
    return SF_ARITHMETIC_FINE;
}

static int mul_school(const sfbigint_t* a, const sfbigint_t* b,
                      sfbigint_t** out) {
    size_t na = used(a), nb = used(b);
    if (na > SIZE_MAX - nb)
        return SF_ARITHMETIC_MEMORY_ERROR;
    sfbigint_t* r = NULL;
    int rc = alloc_number(&r, na + nb);
    if (rc)
        return rc;
    for (size_t i = 0; i < na; ++i) {
        uint64_t carry = 0;
        for (size_t j = 0; j < nb; ++j) {
            uint64_t v = (uint64_t)a->digits[i] * b->digits[j] +
                         r->digits[i + j] + carry;
            r->digits[i + j] = (uint32_t)v;
            carry = v >> 32;
        }
        r->digits[i + nb] = (uint32_t)carry;
    }
    trim(r);
    *out = r;
    return SF_ARITHMETIC_FINE;
}

/* A read-only view; it must never be freed or mutated. */
static sfbigint_t slice(const sfbigint_t* a, size_t start, size_t count) {
    sfbigint_t view = {a->digits + start, count, SF_ARITHMETIC_PLUS};
    view.size = used(&view);
    return view;
}

static void add_offset(sfbigint_t* r, const sfbigint_t* a, size_t offset) {
    uint64_t carry = 0;
    size_t i = 0;
    for (; i < a->size; ++i) {
        uint64_t v = (uint64_t)r->digits[offset + i] + a->digits[i] + carry;
        r->digits[offset + i] = (uint32_t)v;
        carry = v >> 32;
    }
    while (carry) {
        uint64_t v = (uint64_t)r->digits[offset + i] + carry;
        r->digits[offset + i++] = (uint32_t)v;
        carry = v >> 32;
    }
}

static int mul_recursive(const sfbigint_t* a, const sfbigint_t* b,
                         sfbigint_t** out) {
    size_t na = used(a), nb = used(b);
    size_t max = na > nb ? na : nb, min = na < nb ? na : nb;
    if (zero(a) || zero(b))
        return alloc_number(out, 1);
    if (min <= SF_ARITHMETIC_KARATSUBA_THRESHOLD || min <= 3 || max / 2 >= min)
        return mul_school(a, b, out);
    if (na > SIZE_MAX - nb)
        return SF_ARITHMETIC_MEMORY_ERROR;
    size_t m = max / 2;
    sfbigint_t al = slice(a, 0, m), ah = slice(a, m, na - m);
    sfbigint_t bl = slice(b, 0, m), bh = slice(b, m, nb - m);
    sfbigint_t *z0 = NULL, *z2 = NULL, *sa = NULL, *sb = NULL;
    sfbigint_t *product = NULL, *middle = NULL, *z1 = NULL, *r = NULL;
    int rc;
    if ((rc = mul_recursive(&al, &bl, &z0)))
        goto done;
    if ((rc = mul_recursive(&ah, &bh, &z2)))
        goto done;
    if ((rc = add_abs(&al, &ah, &sa)))
        goto done;
    if ((rc = add_abs(&bl, &bh, &sb)))
        goto done;
    if ((rc = mul_recursive(sa, sb, &product)))
        goto done;
    if ((rc = sub_abs(product, z0, &middle)))
        goto done;
    if ((rc = sub_abs(middle, z2, &z1)))
        goto done;
    if ((rc = alloc_number(&r, na + nb)))
        goto done;
    add_offset(r, z0, 0);
    add_offset(r, z1, m);
    add_offset(r, z2, 2 * m);
    trim(r);
    *out = r;
done:
    sfbigint_free(z0);
    sfbigint_free(z2);
    sfbigint_free(sa);
    sfbigint_free(sb);
    sfbigint_free(product);
    sfbigint_free(middle);
    sfbigint_free(z1);
    return rc;
}

int sfbigint_mul_karatsuba(sfbigint_t* a, sfbigint_t* b, sfbigint_t** out) {
    if (!out)
        return SF_ARITHMETIC_INVALID_ARGUMENT;
    if (sfbigint_fine(a) || sfbigint_fine(b))
        return SF_ARITHMETIC_MEMORY_ERROR;
    sfbigint_t* r = NULL;
    int rc = mul_recursive(a, b, &r);
    if (rc)
        return rc;
    r->sign = a->sign ^ b->sign;
    trim(r);
    *out = r;
    return SF_ARITHMETIC_FINE;
}

/* Exact radix-2 FFT over two prime fields (NTT), followed by CRT.
 * 16-bit coefficients avoid floating-point rounding altogether.
 * Both primes have primitive root 3 and support transforms up to 2^23.
 * At that limit each coefficient <= 2^22 * 65535^2 < P1*P2,
 * so the nonnegative integer convolution is uniquely recoverable.
 */
#define NTT_P1 UINT32_C(998244353)
#define NTT_P2 UINT32_C(469762049)
#define NTT_MAX_LENGTH ((size_t)1 << 23)

static uint32_t mod_power(uint32_t a, uint32_t exponent, uint32_t prime) {
    uint64_t result = 1;
    while (exponent) {
        if (exponent & 1u)
            result = result * a % prime;
        a = (uint32_t)((uint64_t)a * a % prime);
        exponent >>= 1;
    }
    return (uint32_t)result;
}

static void ntt(uint32_t* data, size_t n, uint32_t prime, int inverse) {
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) {
            uint32_t temp = data[i];
            data[i] = data[j];
            data[j] = temp;
        }
    }
    for (size_t length = 2; length <= n; length <<= 1) {
        uint32_t root = mod_power(3, (prime - 1) / (uint32_t)length, prime);
        if (inverse)
            root = mod_power(root, prime - 2, prime);
        size_t half = length / 2;
        for (size_t start = 0; start < n; start += length) {
            uint64_t factor = 1;
            for (size_t j = 0; j < half; ++j) {
                uint32_t u = data[start + j];
                uint32_t v =
                    (uint32_t)(data[start + j + half] * factor % prime);
                uint32_t sum = u + v; /* 2*prime < UINT32_MAX */
                data[start + j] = sum >= prime ? sum - prime : sum;
                data[start + j + half] = u >= v ? u - v : u + prime - v;
                factor = factor * root % prime;
            }
        }
    }
    if (inverse) {
        uint32_t scale = mod_power((uint32_t)n, prime - 2, prime);
        for (size_t i = 0; i < n; ++i)
            data[i] = (uint32_t)((uint64_t)data[i] * scale % prime);
    }
}

static void load_half_limbs(uint32_t* data, size_t n, const sfbigint_t* a) {
    memset(data, 0, n * sizeof(*data));
    for (size_t i = 0, count = used(a); i < count; ++i) {
        data[2 * i] = a->digits[i] & UINT32_C(65535);
        data[2 * i + 1] = a->digits[i] >> 16;
    }
}

static void convolution(uint32_t* fa, uint32_t* fb, size_t n, uint32_t prime) {
    ntt(fa, n, prime, 0);
    ntt(fb, n, prime, 0);
    for (size_t i = 0; i < n; ++i)
        fa[i] = (uint32_t)((uint64_t)fa[i] * fb[i] % prime);
    ntt(fa, n, prime, 1);
}

static int mul_fft_abs(const sfbigint_t* a, const sfbigint_t* b,
                       sfbigint_t** out) {
    if (zero(a) || zero(b))
        return alloc_number(out, 1);
    size_t na = used(a), nb = used(b);
    if (na > SIZE_MAX - nb)
        return SF_ARITHMETIC_MEMORY_ERROR;
    /* Beyond the common root-of-unity range, retain exact multiplication. */
    if (na + nb > NTT_MAX_LENGTH / 2)
        return mul_recursive(a, b, out);
    size_t count = 2 * (na + nb), n = 1;
    while (n < count)
        n <<= 1;
    sfbigint_t* r = NULL;
    uint32_t *fa = NULL, *fb = NULL, *first = NULL;
    int rc = alloc_number(&r, na + nb);
    if (rc)
        return rc;
    fa = SF_ARITHMETIC_CALLOC(n, sizeof(*fa));
    fb = SF_ARITHMETIC_CALLOC(n, sizeof(*fb));
    first = SF_ARITHMETIC_CALLOC(count, sizeof(*first));
    if (!fa || !fb || !first) {
        rc = SF_ARITHMETIC_MEMORY_ERROR;
        goto done;
    }
    load_half_limbs(fa, n, a);
    load_half_limbs(fb, n, b);
    convolution(fa, fb, n, NTT_P1);
    memcpy(first, fa, count * sizeof(*first));
    load_half_limbs(fa, n, a);
    load_half_limbs(fb, n, b);
    convolution(fa, fb, n, NTT_P2);
    uint32_t inverse = mod_power(NTT_P1 % NTT_P2, NTT_P2 - 2, NTT_P2);
    uint64_t carry = 0;
    for (size_t i = 0; i < count; ++i) {
        uint32_t reduced = first[i] % NTT_P2;
        uint32_t delta =
            fa[i] >= reduced ? fa[i] - reduced : fa[i] + NTT_P2 - reduced;
        uint64_t multiplier = (uint64_t)delta * inverse % NTT_P2;
        uint64_t coefficient = first[i] + (uint64_t)NTT_P1 * multiplier;
        uint64_t v = coefficient + carry;
        r->digits[i / 2] |= (uint32_t)(v & UINT64_C(65535)) << (16 * (i % 2));
        carry = v >> 16;
    }
    trim(r);
    *out = r;
    r = NULL;
done:
    SF_ARITHMETIC_FREE(fa);
    SF_ARITHMETIC_FREE(fb);
    SF_ARITHMETIC_FREE(first);
    sfbigint_free(r);
    return rc;
}

int sfbigint_mul_fft(sfbigint_t* a, sfbigint_t* b, sfbigint_t** out) {
    if (!out)
        return SF_ARITHMETIC_INVALID_ARGUMENT;
    if (sfbigint_fine(a) || sfbigint_fine(b))
        return SF_ARITHMETIC_MEMORY_ERROR;
    sfbigint_t* r = NULL;
    int rc = mul_fft_abs(a, b, &r);
    if (rc)
        return rc;
    r->sign = a->sign ^ b->sign;
    trim(r);
    *out = r;
    return SF_ARITHMETIC_FINE;
}

int sfbigint_mul(sfbigint_t* a, sfbigint_t* b, sfbigint_t* out) {
    if (sfbigint_fine(a) || sfbigint_fine(b) || sfbigint_fine(out))
        return SF_ARITHMETIC_MEMORY_ERROR;
    sfbigint_t* r = NULL;
    int rc = used(a) >= SF_ARITHMETIC_FFT_THRESHOLD &&
                     used(b) >= SF_ARITHMETIC_FFT_THRESHOLD
                 ? sfbigint_mul_fft(a, b, &r)
                 : sfbigint_mul_karatsuba(a, b, &r);
    if (!rc)
        commit(out, r);
    return rc;
}

/* Leading zeros of a nonzero 32-bit limb; portable C11. */
static unsigned leading_zeros(uint32_t word) {
    unsigned count = 0;
    while (!(word & UINT32_C(0x80000000))) {
        word <<= 1;
        ++count;
    }
    return count;
}

static int div_single(const sfbigint_t* a, uint32_t divisor, sfbigint_t** out) {
    size_t n = used(a);
    sfbigint_t* q = NULL;
    int rc = alloc_number(&q, n);
    if (rc)
        return rc;
    uint64_t remainder = 0;
    for (size_t i = n; i-- > 0;) {
        uint64_t v = (remainder << 32) | a->digits[i];
        q->digits[i] = (uint32_t)(v / divisor);
        remainder = v % divisor;
    }
    trim(q);
    *out = q;
    return SF_ARITHMETIC_FINE;
}

/* Returns true only for a single set bit, with its limb/bit coordinates. */
static int power_of_two(const sfbigint_t* a, size_t* word, unsigned* bit) {
    int found = 0;
    for (size_t i = 0, n = used(a); i < n; ++i) {
        uint32_t v = a->digits[i];
        if (!v)
            continue;
        if (found || (v & (v - 1)))
            return 0;
        found = 1;
        *word = i;
        *bit = 31 - leading_zeros(v);
    }
    return found;
}

static int div_power_two(const sfbigint_t* a, size_t word, unsigned bit,
                         sfbigint_t** out) {
    size_t na = used(a), count = na - word;
    sfbigint_t* q = NULL;
    int rc = alloc_number(&q, count);
    if (rc)
        return rc;
    for (size_t i = 0; i < count; ++i) {
        uint32_t v = a->digits[i + word] >> bit;
        if (bit && i + word + 1 < na)
            v |= a->digits[i + word + 1] << (32 - bit);
        q->digits[i] = v;
    }
    trim(q);
    *out = q;
    return SF_ARITHMETIC_FINE;
}

/* Knuth Algorithm D, base 2^32. Requires |a| > |b| and at least two divisor
 * limbs. D1: normalise so the divisor's top bit is set. D3: estimate and
 * correct one quotient limb from the top dividend limbs. D4-D6:
 * multiply-subtract, then add back if the estimate was still one too high. Only
 * the quotient is requested, so remainder denormalisation (D8) is omitted.
 */
static int div_knuth(const sfbigint_t* a, const sfbigint_t* b,
                     sfbigint_t** out) {
    const uint64_t base = UINT64_C(1) << 32;
    size_t na = used(a), nb = used(b), count = na - nb + 1;
    sfbigint_t *q = NULL, *u = NULL, *v = NULL;
    int rc = alloc_number(&q, count);
    if (rc)
        return rc;
    if ((rc = alloc_number(&u, na + 1)))
        goto done;
    if ((rc = alloc_number(&v, nb)))
        goto done;
    unsigned shift = leading_zeros(b->digits[nb - 1]);
    uint64_t carry = 0;
    for (size_t i = 0; i < na; ++i) {
        uint64_t current = ((uint64_t)a->digits[i] << shift) | carry;
        u->digits[i] = (uint32_t)current;
        carry = current >> 32;
    }
    u->digits[na] = (uint32_t)carry;
    carry = 0;
    for (size_t i = 0; i < nb; ++i) {
        uint64_t current = ((uint64_t)b->digits[i] << shift) | carry;
        v->digits[i] = (uint32_t)current;
        carry = current >> 32;
    }
    for (size_t j = count; j-- > 0;) {
        uint64_t top = v->digits[nb - 1], estimate, remainder;
        if (u->digits[j + nb] == top) {
            /* The raw estimate could be base or base+1: saturate before
             * division. */
            estimate = base - 1;
            remainder = (uint64_t)u->digits[j + nb - 1] + top;
        } else {
            uint64_t numerator =
                ((uint64_t)u->digits[j + nb] << 32) | u->digits[j + nb - 1];
            estimate = numerator / top;
            remainder = numerator % top;
        }
        while (remainder < base &&
               estimate * v->digits[nb - 2] >
                   (remainder << 32) + u->digits[j + nb - 2]) {
            --estimate;
            remainder += top;
        }
        uint64_t borrow = 0;
        for (size_t i = 0; i < nb; ++i) {
            uint64_t product = estimate * v->digits[i] + borrow;
            uint32_t low = (uint32_t)product, previous = u->digits[j + i];
            u->digits[j + i] = previous - low;
            borrow = (product >> 32) + (previous < low);
        }
        uint32_t previous = u->digits[j + nb];
        u->digits[j + nb] = (uint32_t)((uint64_t)previous - borrow);
        if (previous < borrow) {
            --estimate;
            carry = 0;
            for (size_t i = 0; i < nb; ++i) {
                uint64_t sum =
                    (uint64_t)u->digits[j + i] + v->digits[i] + carry;
                u->digits[j + i] = (uint32_t)sum;
                carry = sum >> 32;
            }
            u->digits[j + nb] += (uint32_t)carry;
        }
        q->digits[j] = (uint32_t)estimate;
    }
    trim(q);
    *out = q;
    q = NULL;
done:
    sfbigint_free(q);
    sfbigint_free(u);
    sfbigint_free(v);
    return rc;
}

int sfbigint_div(sfbigint_t* a, sfbigint_t* b, sfbigint_t* out) {
    if (sfbigint_fine(a) || sfbigint_fine(b) || sfbigint_fine(out))
        return SF_ARITHMETIC_MEMORY_ERROR;
    if (zero(b))
        return SF_ARITHMETIC_INVALID_ARGUMENT;
    sfbigint_t* q = NULL;
    int comparison = cmp_abs(a, b), rc;
    size_t word = 0;
    unsigned bit = 0;
    if (comparison <= 0) {
        rc = alloc_number(&q, 1);
        if (!rc)
            q->digits[0] = comparison == 0 ? 1 : 0;
    } else if (power_of_two(b, &word, &bit)) {
        rc = div_power_two(a, word, bit, &q);
    } else if (used(b) == 1) {
        rc = div_single(a, b->digits[0], &q);
    } else {
        rc = div_knuth(a, b, &q);
    }
    if (rc)
        return rc;
    q->sign = a->sign ^ b->sign;
    trim(q);
    commit(out, q);
    return SF_ARITHMETIC_FINE;
}
