#include <stdio.h>

#include "sf_arithmetic.h"

static void print_hex(sfbigint_t* a) {
    putchar(a->sign ? '-' : '+');
    for (size_t i = a->size; i-- > 0;)
        printf("%08x", a->digits[i]);
}

/* Large reference cases use hex to avoid measuring decimal conversion. */
static int parse_input(sfbigint_t** out, const char* text) {
    int negative = text[0] == '-';
    const char* digits = text + negative;
    if (digits[0] != '0' || digits[1] != 'x')
        return sfbigint_strtosfbigint(out, text, strlen(text));
    digits += 2;
    size_t n = strlen(digits);
    if (!n)
        return SF_ARITHMETIC_INVALID_ARGUMENT;
    sfbigint_t* value = NULL;
    int rc = sfbigint_create(&value, (uint32_t)((n + 7) / 8));
    if (rc)
        return rc;
    for (size_t i = 0; i < n; ++i) {
        char c = digits[n - 1 - i];
        unsigned digit;
        if (c >= '0' && c <= '9')
            digit = (unsigned)(c - '0');
        else if (c >= 'a' && c <= 'f')
            digit = (unsigned)(c - 'a' + 10);
        else {
            sfbigint_free(value);
            return SF_ARITHMETIC_INVALID_ARGUMENT;
        }
        value->digits[i / 8] |= (uint32_t)digit << (4 * (i % 8));
    }
    value->sign = negative;
    sfbigint_normalise(value);
    *out = value;
    return 0;
}

int main(void) {
    char op[24], left[131080], right[131080];
    while (scanf("%23s %131079s %131079s", op, left, right) == 3) {
        sfbigint_t *a = NULL, *b = NULL, *r = NULL, *product = NULL;
        char* str = NULL;
        int rc = parse_input(&a, left);
        if (!rc)
            rc = parse_input(&b, right);
        if (!rc)
            rc = sfbigint_create(&r, 1);
        if (rc) {
            sfbigint_free(a);
            sfbigint_free(b);
            sfbigint_free(r);
            return 2;
        }
        sfbigint_t* dst = r;
        size_t len = strlen(op);
        if (op[len - 1] == 'A') {
            dst = a;
            op[--len] = '\0';
        }
        if (op[len - 1] == 'B') {
            dst = b;
            op[--len] = '\0';
        }
        if (!strcmp(op, "add"))
            rc = sfbigint_add(a, b, dst);
        else if (!strcmp(op, "sub"))
            rc = sfbigint_sub(a, b, dst);
        else if (!strcmp(op, "mul"))
            rc = sfbigint_mul(a, b, dst);
        else if (!strcmp(op, "div"))
            rc = sfbigint_div(a, b, dst);
        else if (!strcmp(op, "karatsuba")) {
            rc = sfbigint_mul_karatsuba(a, b, &product);
            dst = product;
        } else if (!strcmp(op, "fft")) {
            rc = sfbigint_mul_fft(a, b, &product);
            dst = product;
        } else if (!strcmp(op, "left")) {
            dst = a;
            rc = sfbigint_lshift(a, (size_t)strtoull(right, NULL, 10));
        } else if (!strcmp(op, "right")) {
            dst = a;
            rc = sfbigint_rshift(a, (size_t)strtoull(right, NULL, 10));
        } else if (!strcmp(op, "mul10")) {
            dst = a;
            rc = sfbigint_mul10(a);
        } else if (!strcmp(op, "div10")) {
            uint32_t rem = 0;
            dst = a;
            rc = sfbigint_divmod10(a, &rem);
            b->digits[0] = rem;
            b->size = 1;
            b->sign = 0;
        } else if (!strcmp(op, "compare")) {
            rc = sfbigint_compare(a, b);
            dst = a;
        } else if (!strcmp(op, "parse"))
            dst = a;
        else if (!strcmp(op, "text")) {
            rc = sfbigint_tostring(a, &str);
            dst = a;
        } else
            return 3;
        printf("%d ", rc);
        if (dst)
            print_hex(dst);
        else
            printf("+0");
        putchar(' ');
        print_hex(a);
        putchar(' ');
        print_hex(b);
        printf(" %s\n", str ? str : "_");
        SF_ARITHMETIC_FREE(str);
        sfbigint_free(product);
        sfbigint_free(a);
        sfbigint_free(b);
        sfbigint_free(r);
    }
    return ferror(stdin) ? 4 : 0;
}
