

extern "C" {
#include "sf_arithmetic.h"
}

sfbigint_t* generate(SF_ARITHMETIC_DIGITS_T size) {
    sfbigint_t* obj = NULL;
    sfbigint_create(&obj, size);

    for (SF_ARITHMETIC_SIZE_T i = 0; i < size; ++i) {
        obj->digits[i] = rand();
    }

    obj->sign = rand() % 2;

    return obj;
}

void test_add(SF_ARITHMETIC_SIZE_T size, SF_ARITHMETIC_SIZE_T obj_size) {
    sfbigint_t** firsts =
        (sfbigint_t**)SF_ARITHMETIC_CALLOC(size, sizeof(sfbigint_t));
    sfbigint_t** seconds =
        (sfbigint_t**)SF_ARITHMETIC_CALLOC(size, sizeof(sfbigint_t));
    sfbigint_t** results =
        (sfbigint_t**)SF_ARITHMETIC_CALLOC(size, sizeof(sfbigint_t));

    for (SF_ARITHMETIC_SIZE_T i = 0; i < size; ++i) {
        firsts[i] = generate(obj_size);
        seconds[i] = generate(obj_size);
    }

    for (SF_ARITHMETIC_SIZE_T i = 0; i < size; ++i) {
        sfbigint_add(firsts[i], seconds[i], results[i]);
    }
}

int main() {
    test_add(1e6, 100);

    return 0;
}