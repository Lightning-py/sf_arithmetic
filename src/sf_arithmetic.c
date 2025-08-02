#ifndef SF_ARITHMETIC_C
#define SF_ARITHMETIC_C

#include "sf_arithmetic.h"

SF_ARITHMETIC_STATUS_CODE sfbigint_fine(sfbigint_t* obj) { return (obj && obj->digits && obj->size > 0) ? SF_ARITHMETIC_FINE : SF_ARITHMETIC_MEMORY_ERROR; }

SF_ARITHMETIC_STATUS_CODE sfbigint_create(sfbigint_t **obj, SF_ARITHMETIC_DIGITS_T size) {
    if (size == 0) return SF_ARITHMETIC_INVALID_ARGUMENT;

    *obj = (sfbigint_t*) SF_ARITHMETIC_MALLOC(sizeof(sfbigint_t));

    if (!*obj) return SF_ARITHMETIC_MEMORY_ERROR;

    (*obj)->digits = SF_ARITHMETIC_CALLOC(size, sizeof(SF_ARITHMETIC_DIGITS_T));

    if (!(*obj)->digits) return SF_ARITHMETIC_MEMORY_ERROR;

    (*obj)->size = size;
    (*obj)->sign = SF_ARITHMETIC_PLUS;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_free(sfbigint_t *obj) {
    if (sfbigint_fine(obj) == SF_ARITHMETIC_FINE) {
        SF_ARITHMETIC_FREE(obj->digits);
        SF_ARITHMETIC_FREE(obj);
        return SF_ARITHMETIC_FINE;
    } 

    return SF_ARITHMETIC_MEMORY_ERROR;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_setzero(sfbigint_t* obj) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE) return SF_ARITHMETIC_MEMORY_ERROR;

    obj->sign = SF_ARITHMETIC_PLUS;

    if (obj->size != 1) {
        SF_ARITHMETIC_DIGITS_T* new_digits = SF_ARITHMETIC_REALLOC(obj->digits, sizeof(SF_ARITHMETIC_DIGITS_T) * 1);

        if (!new_digits) return SF_ARITHMETIC_MEMORY_ERROR;
        obj->digits = new_digits;
        obj->digits[0] = 0;
        obj->sign = 1;
    } else obj->digits[0] = 0;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_copy(sfbigint_t* obj, sfbigint_t** copy) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE) return SF_ARITHMETIC_MEMORY_ERROR;
    
    SF_ARITHMETIC_SIGN_T rc = sfbigint_create(copy, obj->size);
    if (rc != SF_ARITHMETIC_FINE) return rc;
    
    memcpy((*copy)->digits, obj->digits, sizeof(SF_ARITHMETIC_DIGITS_T) * obj->size);
    (*copy)->size = obj->size;

    return SF_ARITHMETIC_FINE;
}


SF_ARITHMETIC_STATUS_CODE sfbigint_swap(sfbigint_t* first, sfbigint_t* second) {
    if (sfbigint_fine(first) != SF_ARITHMETIC_FINE || sfbigint_fine(second) != SF_ARITHMETIC_FINE) return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_DIGITS_T* digits_temp = first->digits;
    SF_ARITHMETIC_SIZE_T size_temp = first->size;
    SF_ARITHMETIC_SIGN_T sign_temp = first->sign;
    
    first->digits = second->digits;
    first->size = second->size;
    first->sign = second->sign;

    second->digits = digits_temp;
    second->size = size_temp;
    second->sign = sign_temp;

    return SF_ARITHMETIC_FINE;
}


SF_ARITHMETIC_STATUS_CODE sfbigint_normalise(sfbigint_t* obj) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE) return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_SIZE_T new_size = obj->size;

    for (SF_ARITHMETIC_SIZE_T i = new_size - 1; i < obj->size && obj->digits[i] == 0; i--) new_size--;

    SF_ARITHMETIC_DIGITS_T* new_digits = SF_ARITHMETIC_REALLOC(obj->digits, sizeof(SF_ARITHMETIC_DIGITS_T) * new_size);

    if (!new_digits) return SF_ARITHMETIC_MEMORY_ERROR;

    obj->digits = new_digits;
    obj->size = new_size;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_SIGN_T sfbigint_compare(sfbigint_t* first, sfbigint_t* second) {
    if (sfbigint_fine(first) != SF_ARITHMETIC_FINE || sfbigint_fine(second) != SF_ARITHMETIC_FINE) return SF_ARITHMETIC_MEMORY_ERROR;
    
    SF_ARITHMETIC_SIZE_T rc1 = sfbigint_normalise(first);
    SF_ARITHMETIC_SIZE_T rc2 = sfbigint_normalise(second);

    if (rc1 != SF_ARITHMETIC_FINE || rc2 != SF_ARITHMETIC_FINE) return SF_ARITHMETIC_MEMORY_ERROR;

    if (first->size > second->size) return 1;
    if (first->size < second->size) return 2;

    SF_ARITHMETIC_SIZE_T i = first->size - 1;

    while (i < first->size && first->digits[i] == second->digits[i]) i--;
    
    if (i > first->size) return 0;
    else if (first->digits[i] > second->digits[i]) return 1;
    return 2;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_add_(sfbigint_t* first, sfbigint_t* second, sfbigint_t* res) {
    if ( !(sfbigint_fine(first) == SF_ARITHMETIC_FINE && sfbigint_fine(second) == SF_ARITHMETIC_FINE) ) return SF_ARITHMETIC_MEMORY_ERROR;
    
    SF_ARITHMETIC_SIZE_T max_size = SF_MAX(first->size, second->size);
   
    if (res->size != max_size) {
        SF_ARITHMETIC_FREE(res->digits);
        res->digits = SF_ARITHMETIC_CALLOC(max_size, sizeof(SF_ARITHMETIC_DIGITS_T));
    }

    if (!res->digits) return SF_ARITHMETIC_MEMORY_ERROR;
    res->size = max_size;

    SF_ARITHMETIC_DIGITS_T remainder = 0;
    for (SF_ARITHMETIC_SIZE_T i = 0; i < max_size; ++i) {
        SF_ARITHMETIC_BIGDIGITS_T temp = (SF_ARITHMETIC_BIGDIGITS_T) remainder;

        if (i < first->size) temp += (SF_ARITHMETIC_BIGDIGITS_T) first->digits[i];
        if (i < second->size) temp += (SF_ARITHMETIC_BIGDIGITS_T) second->digits[i];

        res->digits[i] = temp & 0xFFFFFFFF;
        remainder = (SF_ARITHMETIC_DIGITS_T) (temp >> 32);
    }

    if (remainder) {
        SF_ARITHMETIC_DIGITS_T* temp = SF_ARITHMETIC_REALLOC(res->digits, (res->size + 1) * sizeof(SF_ARITHMETIC_DIGITS_T));

        if (!temp) {
            sfbigint_free(res);
            return SF_ARITHMETIC_MEMORY_ERROR;
        }

        temp[res->size] = remainder;
        res->digits = temp;
        res->size++;
    }


    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_sub_(sfbigint_t* first, sfbigint_t* second, sfbigint_t* res) {
    // сравнение чисел и замена

    SF_ARITHMETIC_SIGN_T rc_compare = sfbigint_compare(first, second);
    if (rc_compare == SF_ARITHMETIC_MEMORY_ERROR) return rc_compare;

    SF_ARITHMETIC_SIZE_T max_size = first->size;

    if (rc_compare == 2) {
        res->sign = SF_ARITHMETIC_MINUS;

        SF_ARITHMETIC_SIGN_T rc_swap = sfbigint_swap(first, second);
        if (rc_swap != SF_ARITHMETIC_FINE) return rc_swap;

        max_size = second->size;
    }

    SF_ARITHMETIC_DIGITS_T* new_digits = SF_ARITHMETIC_REALLOC(res->digits, sizeof(SF_ARITHMETIC_DIGITS_T) * max_size);
    if (!new_digits) return SF_ARITHMETIC_MEMORY_ERROR;

    res->digits = new_digits;
    res->size = first->size;

    // алгоритм вычитания из большего числа меньшего
    // гарантированно first > second, вычитаем first - second

    for (SF_ARITHMETIC_SIZE_T i = 0; i < second->size; ++i) {
        SF_ARITHMETIC_BIGDIGITS_T second_digit = second->digits[i];
        SF_ARITHMETIC_BIGDIGITS_T first_digit = first->digits[i];

        if (first_digit < second_digit) {
            first_digit += (SF_ARITHMETIC_BIGDIGITS_T) 1 << 32;
            first->digits[i + 1]--;
        }

        res->digits[i] = first_digit - second_digit;
    }

    int rc_normalise = sfbigint_normalise(res);
    if (rc_normalise != SF_ARITHMETIC_FINE) return rc_normalise;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_add(sfbigint_t *first, sfbigint_t *second, sfbigint_t *res) {
    if (sfbigint_fine(first) != SF_ARITHMETIC_FINE || sfbigint_fine(second) != SF_ARITHMETIC_FINE || sfbigint_fine(res) != SF_ARITHMETIC_FINE) return SF_ARITHMETIC_MEMORY_ERROR;

    if (first->sign == SF_ARITHMETIC_PLUS) {
        if (second->sign == SF_ARITHMETIC_PLUS) return sfbigint_add_(first, second, res);
        else return sfbigint_sub_(first, second, res);
    } else {
        if (second->sign == SF_ARITHMETIC_PLUS) return sfbigint_sub_(second, first, res);
        else {
            first->sign = SF_ARITHMETIC_PLUS;
            second->sign = SF_ARITHMETIC_PLUS;

            SF_ARITHMETIC_SIGN_T rc = sfbigint_add_(first, second, res);
            res->sign = SF_ARITHMETIC_MINUS;

            return rc;
        }
    }
}

SF_ARITHMETIC_STATUS_CODE sfbigint_sub(sfbigint_t *first, sfbigint_t *second, sfbigint_t *res) {
    if (sfbigint_fine(first) != SF_ARITHMETIC_FINE || sfbigint_fine(second) != SF_ARITHMETIC_FINE || sfbigint_fine(res) != SF_ARITHMETIC_FINE) return SF_ARITHMETIC_MEMORY_ERROR;

    if (first->sign == SF_ARITHMETIC_PLUS) {
        if (second->sign == SF_ARITHMETIC_PLUS) return sfbigint_sub_(first, second, res);
        else return sfbigint_add_(first, second, res);
    } else {
        if (second->sign == SF_ARITHMETIC_MINUS) return sfbigint_sub_(second, first, res);
        else {
            first->sign = SF_ARITHMETIC_PLUS;
            second->sign = SF_ARITHMETIC_PLUS;

            SF_ARITHMETIC_SIGN_T rc = sfbigint_add_(first, second, res);
            res->sign = SF_ARITHMETIC_MINUS;

            return rc;
        }
    }
}



#endif // SF_ARITHMETIC_C