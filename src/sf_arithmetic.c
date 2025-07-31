#ifndef SF_ARITHMETIC_C
#define SF_ARITHMETIC_C

#include "sf_arithmetic.h"

SF_ARITHMETIC_STATUS_CODE sfbigint_create(sfbigint_t *obj, SF_ARITHMETIC_DIGITS_T size) {
    obj = (sfbigint_t*) SF_ARITHMETIC_MALLOC(sizeof(sfbigint_t));

    if (!obj) return SF_MEMORY_ERROR;

    obj->digits = SF_ARITHMETIC_CALLOC(size, sizeof(SF_ARITHMETIC_DIGITS_T));

    if (!obj->digits) return SF_MEMORY_ERROR;

    obj->size = size;
    obj->sign = SF_ARITHMETIC_PLUS;

    return SF_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_free(sfbigint_t *obj) {
    if (!obj || !obj->digits) return SF_MEMORY_ERROR;

    SF_ARITHMETIC_FREE(obj->digits);
    SF_ARITHMETIC_FREE(obj);

    return SF_FINE;
}

#endif // SF_ARITHMETIC_C