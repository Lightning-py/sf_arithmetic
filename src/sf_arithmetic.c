#ifndef SF_ARITHMETIC_C
#define SF_ARITHMETIC_C

#include "sf_arithmetic.h"

SF_ARITHMETIC_STATUS_CODE sfbigint_fine(sfbigint_t* obj) {
    return (obj && obj->digits && obj->size > 0) ? SF_ARITHMETIC_FINE
                                                 : SF_ARITHMETIC_MEMORY_ERROR;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_iszero(sfbigint_t* obj) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_STATUS_CODE flag = 1;
    for (SF_ARITHMETIC_SIZE_T i = 0; (i < obj->size) && flag; ++i)
        flag = obj->digits[i] == 0;

    return flag;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_create(sfbigint_t** obj,
                                          SF_ARITHMETIC_DIGITS_T size) {
    if (size == 0) return SF_ARITHMETIC_INVALID_ARGUMENT;

    *obj = (sfbigint_t*)SF_ARITHMETIC_MALLOC(sizeof(sfbigint_t));

    if (!*obj) return SF_ARITHMETIC_MEMORY_ERROR;

    (*obj)->digits = SF_ARITHMETIC_CALLOC(size, SF_ARITHMETIC_DIGITS_T_SIZE);

    if (!(*obj)->digits) return SF_ARITHMETIC_MEMORY_ERROR;

    (*obj)->size = size;
    (*obj)->sign = SF_ARITHMETIC_PLUS;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_free(sfbigint_t* obj) {
    if (sfbigint_fine(obj) == SF_ARITHMETIC_FINE) {
        SF_ARITHMETIC_FREE(obj->digits);
        SF_ARITHMETIC_FREE(obj);
        return SF_ARITHMETIC_FINE;
    }

    return SF_ARITHMETIC_MEMORY_ERROR;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_setzero(sfbigint_t* obj) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    obj->sign = SF_ARITHMETIC_PLUS;

    if (obj->size != 1) {
        SF_ARITHMETIC_DIGITS_T* new_digits =
            SF_ARITHMETIC_REALLOC(obj->digits, SF_ARITHMETIC_DIGITS_T_SIZE * 1);

        if (!new_digits) return SF_ARITHMETIC_MEMORY_ERROR;
        obj->digits = new_digits;
        obj->digits[0] = 0;
        obj->sign = 1;
    } else
        obj->digits[0] = 0;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_copy(sfbigint_t* obj, sfbigint_t** copy) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_SIGN_T rc = sfbigint_create(copy, obj->size);
    if (rc != SF_ARITHMETIC_FINE) return rc;

    memcpy((*copy)->digits, obj->digits,
           SF_ARITHMETIC_DIGITS_T_SIZE * obj->size);
    (*copy)->size = obj->size;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_swap(sfbigint_t* first, sfbigint_t* second) {
    if (sfbigint_fine(first) != SF_ARITHMETIC_FINE ||
        sfbigint_fine(second) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

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
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_SIZE_T new_size = obj->size;

    for (SF_ARITHMETIC_SIZE_T i = new_size - 1;
         i < obj->size && obj->digits[i] == 0; i--)
        new_size--;

    new_size = new_size == 0 ? 1 : new_size;

    SF_ARITHMETIC_DIGITS_T* new_digits = SF_ARITHMETIC_REALLOC(
        obj->digits, SF_ARITHMETIC_DIGITS_T_SIZE * new_size);

    if (!new_digits) return SF_ARITHMETIC_MEMORY_ERROR;

    obj->digits = new_digits;
    obj->size = new_size;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_SIGN_T sfbigint_compare(sfbigint_t* first, sfbigint_t* second) {
    if (sfbigint_fine(first) != SF_ARITHMETIC_FINE ||
        sfbigint_fine(second) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_SIZE_T rc1 = sfbigint_normalise(first);
    SF_ARITHMETIC_SIZE_T rc2 = sfbigint_normalise(second);

    if (rc1 != SF_ARITHMETIC_FINE || rc2 != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    if (first->size > second->size) return 1;
    if (first->size < second->size) return 2;

    SF_ARITHMETIC_SIZE_T i = first->size - 1;

    while (i < first->size && first->digits[i] == second->digits[i]) i--;

    if (i > first->size)
        return 0;
    else if (first->digits[i] > second->digits[i])
        return 1;
    return 2;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_lshift(sfbigint_t* obj,
                                          SF_ARITHMETIC_SIZE_T times) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_SIZE_T full_ints = times / SF_ARITHMETIC_DIGITS_T_SIZE_BITS,
                         part_ints = times % SF_ARITHMETIC_DIGITS_T_SIZE_BITS;

    SF_ARITHMETIC_SIZE_T new_size =
        obj->size + full_ints + (part_ints != 0 ? 1 : 0);

    if (full_ints == 0 && part_ints == 0) return SF_ARITHMETIC_FINE;

    SF_ARITHMETIC_DIGITS_T* new_digits =
        SF_ARITHMETIC_CALLOC(new_size, SF_ARITHMETIC_DIGITS_T_SIZE);
    if (!new_digits) return SF_ARITHMETIC_MEMORY_ERROR;

    memcpy(new_digits + full_ints, obj->digits,
           obj->size * SF_ARITHMETIC_DIGITS_T_SIZE);

    SF_ARITHMETIC_FREE(obj->digits);
    obj->digits = new_digits;

    if (part_ints != 0) {
        SF_ARITHMETIC_DIGITS_T remainder = 0;

        for (SF_ARITHMETIC_SIZE_T i = 0; i < new_size; ++i) {
            SF_ARITHMETIC_BIGDIGITS_T temp =
                ((SF_ARITHMETIC_BIGDIGITS_T)obj->digits[i] << part_ints) |
                remainder;

            obj->digits[i] =
                (SF_ARITHMETIC_DIGITS_T)temp & SF_ARITHMETIC_DIGITS_T_FULL_MASK;
            remainder =
                (SF_ARITHMETIC_DIGITS_T)((temp &
                                          SF_ARITHMETIC_BIGDIGITS_LEFTHALF_MASK) >>
                                         SF_ARITHMETIC_DIGITS_T_SIZE_BITS);
        }
    }

    SF_ARITHMETIC_STATUS_CODE rc = sfbigint_normalise(obj);
    if (rc != SF_ARITHMETIC_FINE) return rc;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_rshift(sfbigint_t* obj,
                                          SF_ARITHMETIC_SIZE_T times) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_SIZE_T full_ints = times / SF_ARITHMETIC_DIGITS_T_SIZE_BITS,
                         part_ints = times % SF_ARITHMETIC_DIGITS_T_SIZE_BITS;

    SF_ARITHMETIC_SIZE_T new_size = obj->size - full_ints;

    if (full_ints == 0 && part_ints == 0) return SF_ARITHMETIC_FINE;

    SF_ARITHMETIC_DIGITS_T* new_digits =
        SF_ARITHMETIC_CALLOC(new_size, SF_ARITHMETIC_DIGITS_T_SIZE);
    if (!new_digits) return SF_ARITHMETIC_MEMORY_ERROR;

    memcpy(new_digits, obj->digits + full_ints,
           new_size * SF_ARITHMETIC_DIGITS_T_SIZE);

    SF_ARITHMETIC_FREE(obj->digits);
    obj->digits = new_digits;

    if (part_ints != 0) {
        SF_ARITHMETIC_DIGITS_T remainder = 0;

        for (SF_ARITHMETIC_SIZE_T i = new_size - 1; i < new_size; --i) {
            SF_ARITHMETIC_BIGDIGITS_T temp =
                ((SF_ARITHMETIC_BIGDIGITS_T)obj->digits[i])
                << (SF_ARITHMETIC_DIGITS_T_SIZE_BITS - part_ints);

            obj->digits[i] =
                (SF_ARITHMETIC_DIGITS_T)(((temp &
                                           SF_ARITHMETIC_BIGDIGITS_LEFTHALF_MASK) >>
                                          SF_ARITHMETIC_DIGITS_T_SIZE_BITS) |
                                         remainder);

            remainder =
                (SF_ARITHMETIC_DIGITS_T)(temp &
                                         SF_ARITHMETIC_DIGITS_T_FULL_MASK);
        }
    }

    SF_ARITHMETIC_STATUS_CODE rc = sfbigint_normalise(obj);
    if (rc != SF_ARITHMETIC_FINE) return rc;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_divmod10(sfbigint_t* obj,
                                            SF_ARITHMETIC_DIGITS_T* res) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_BIGDIGITS_T remainder = 0;
    for (int i = obj->size; i >= 0; i--) {
        SF_ARITHMETIC_BIGDIGITS_T current =
            remainder << SF_ARITHMETIC_DIGITS_T_SIZE_BITS |
            ((SF_ARITHMETIC_BIGDIGITS_T)obj->digits[i]);

        obj->digits[i] = (SF_ARITHMETIC_DIGITS_T)(current / 10);
        remainder = current % 10;
    }

    SF_ARITHMETIC_STATUS_CODE rc = sfbigint_normalise(obj);

    // если результат неудовлетворительный не затираем переменную
    if (rc != SF_ARITHMETIC_FINE) return rc;

    *res = remainder;
    return rc;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_mul10(sfbigint_t* obj) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_BIGDIGITS_T remainder = 0;
    for (SF_ARITHMETIC_SIZE_T i = 0; i < obj->size; ++i) {
        SF_ARITHMETIC_BIGDIGITS_T current =
            ((SF_ARITHMETIC_BIGDIGITS_T)obj->digits[i] * 10) | remainder;

        obj->digits[i] =
            (SF_ARITHMETIC_DIGITS_T)(current &
                                     SF_ARITHMETIC_DIGITS_T_FULL_MASK);
        remainder = (current & SF_ARITHMETIC_BIGDIGITS_LEFTHALF_MASK) >>
                    SF_ARITHMETIC_DIGITS_T_SIZE_BITS;
    }

    if (remainder > 0) {
        SF_ARITHMETIC_DIGITS_T* new_digits = SF_ARITHMETIC_REALLOC(
            obj->digits, SF_ARITHMETIC_DIGITS_T_SIZE * (obj->size + 1));

        if (!new_digits) return SF_ARITHMETIC_MEMORY_ERROR;

        obj->digits = new_digits;
        obj->digits[obj->size] = remainder;
        obj->size++;
    }

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_strtosfbigint(sfbigint_t** obj,
                                                 SF_ARITHMETIC_CHAR_T* str,
                                                 SF_ARITHMETIC_SIZE_T len) {
    sfbigint_t* obj_ = NULL;
    SF_ARITHMETIC_STATUS_CODE rc_create = sfbigint_create(&obj_, 1);

    if (rc_create != SF_ARITHMETIC_FINE) return rc_create;

    for (SF_ARITHMETIC_SIZE_T i = 0; i < len; ++i) {
        obj_->digits[0] += str[i] - '0';

        if (i != len - 1) {
            SF_ARITHMETIC_STATUS_CODE rc_mul = sfbigint_mul10(obj_);

            if (rc_mul != SF_ARITHMETIC_FINE) {
                sfbigint_free(obj_);
                return rc_mul;
            }
        }
    }

    *obj = obj_;
    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_tostring(sfbigint_t* obj,
                                            SF_ARITHMETIC_CHAR_T** res) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_CHAR_T* str =
        SF_ARITHMETIC_CALLOC(10, sizeof(SF_ARITHMETIC_CHAR_T));
    SF_ARITHMETIC_SIZE_T str_size = 10;

    sfbigint_t* obj_ = NULL;
    SF_ARITHMETIC_STATUS_CODE rc_copy = sfbigint_copy(obj, &obj_);

    if (rc_copy != SF_ARITHMETIC_FINE) return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_SIZE_T num_count = 0;
    while (!sfbigint_iszero(obj_)) {
        SF_ARITHMETIC_DIGITS_T num = 0;
        SF_ARITHMETIC_STATUS_CODE rc_divmod = sfbigint_divmod10(obj_, &num);

        if (rc_divmod != SF_ARITHMETIC_FINE) return rc_divmod;

        if (num_count + 1 >= str_size) {
            str_size *= 2;
            SF_ARITHMETIC_CHAR_T* temp = SF_ARITHMETIC_REALLOC(
                str, sizeof(SF_ARITHMETIC_CHAR_T) * str_size);

            if (!temp) {
                SF_ARITHMETIC_FREE(str);
                return SF_ARITHMETIC_MEMORY_ERROR;
            }

            str = temp;
        }

        str[num_count] = '0' + num;
    }

    for (SF_ARITHMETIC_SIZE_T i = 0; i < num_count / 2; ++i) {
        SF_ARITHMETIC_CHAR_T temp = str[i];
        str[i] = str[num_count - i];
        str[num_count - i] = temp;
    }

    num_count++;

    str[num_count] = '\0';

    *res = str;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE __sfbigint_add__(sfbigint_t* first,
                                           sfbigint_t* second,
                                           sfbigint_t* res) {
    if (!(sfbigint_fine(first) == SF_ARITHMETIC_FINE &&
          sfbigint_fine(second) == SF_ARITHMETIC_FINE))
        return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_SIZE_T max_size = SF_MAX(first->size, second->size);

    if (res->size != max_size) {
        SF_ARITHMETIC_FREE(res->digits);
        res->digits =
            SF_ARITHMETIC_CALLOC(max_size, SF_ARITHMETIC_DIGITS_T_SIZE);
    }

    if (!res->digits) return SF_ARITHMETIC_MEMORY_ERROR;
    res->size = max_size;

    SF_ARITHMETIC_DIGITS_T remainder = 0;
    for (SF_ARITHMETIC_SIZE_T i = 0; i < max_size; ++i) {
        SF_ARITHMETIC_BIGDIGITS_T temp = (SF_ARITHMETIC_BIGDIGITS_T)remainder;

        if (i < first->size)
            temp += (SF_ARITHMETIC_BIGDIGITS_T)first->digits[i];
        if (i < second->size)
            temp += (SF_ARITHMETIC_BIGDIGITS_T)second->digits[i];

        res->digits[i] = temp & SF_ARITHMETIC_DIGITS_T_FULL_MASK;
        remainder =
            (SF_ARITHMETIC_DIGITS_T)(temp >> SF_ARITHMETIC_DIGITS_T_SIZE_BITS);
    }

    if (remainder) {
        SF_ARITHMETIC_DIGITS_T* temp = SF_ARITHMETIC_REALLOC(
            res->digits, (res->size + 1) * SF_ARITHMETIC_DIGITS_T_SIZE);

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

SF_ARITHMETIC_STATUS_CODE __sfbigint_sub__(sfbigint_t* first,
                                           sfbigint_t* second,
                                           sfbigint_t* res) {
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

    SF_ARITHMETIC_DIGITS_T* new_digits = SF_ARITHMETIC_REALLOC(
        res->digits, SF_ARITHMETIC_DIGITS_T_SIZE * max_size);
    if (!new_digits) return SF_ARITHMETIC_MEMORY_ERROR;

    res->digits = new_digits;
    res->size = first->size;

    // алгоритм вычитания из большего числа меньшего
    // гарантированно first > second, вычитаем first - second

    for (SF_ARITHMETIC_SIZE_T i = 0; i < second->size; ++i) {
        SF_ARITHMETIC_BIGDIGITS_T second_digit = second->digits[i];
        SF_ARITHMETIC_BIGDIGITS_T first_digit = first->digits[i];

        if (first_digit < second_digit) {
            first_digit += (SF_ARITHMETIC_BIGDIGITS_T)1
                           << SF_ARITHMETIC_DIGITS_T_SIZE_BITS;
            first->digits[i + 1]--;
        }

        res->digits[i] = first_digit - second_digit;
    }

    int rc_normalise = sfbigint_normalise(res);
    if (rc_normalise != SF_ARITHMETIC_FINE) return rc_normalise;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_add(sfbigint_t* first, sfbigint_t* second,
                                       sfbigint_t* res) {
    if (sfbigint_fine(first) != SF_ARITHMETIC_FINE ||
        sfbigint_fine(second) != SF_ARITHMETIC_FINE ||
        sfbigint_fine(res) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    if (first->sign == SF_ARITHMETIC_PLUS) {
        if (second->sign == SF_ARITHMETIC_PLUS)
            return __sfbigint_add__(first, second, res);
        else
            return __sfbigint_sub__(first, second, res);
    } else {
        if (second->sign == SF_ARITHMETIC_PLUS)
            return __sfbigint_sub__(second, first, res);
        else {
            first->sign = SF_ARITHMETIC_PLUS;
            second->sign = SF_ARITHMETIC_PLUS;

            SF_ARITHMETIC_SIGN_T rc = __sfbigint_add__(first, second, res);
            res->sign = SF_ARITHMETIC_MINUS;

            return rc;
        }
    }
}

SF_ARITHMETIC_STATUS_CODE sfbigint_sub(sfbigint_t* first, sfbigint_t* second,
                                       sfbigint_t* res) {
    if (sfbigint_fine(first) != SF_ARITHMETIC_FINE ||
        sfbigint_fine(second) != SF_ARITHMETIC_FINE ||
        sfbigint_fine(res) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    if (first->sign == SF_ARITHMETIC_PLUS) {
        if (second->sign == SF_ARITHMETIC_PLUS)
            return __sfbigint_sub__(first, second, res);
        else
            return __sfbigint_add__(first, second, res);
    } else {
        if (second->sign == SF_ARITHMETIC_MINUS)
            return __sfbigint_sub__(second, first, res);
        else {
            first->sign = SF_ARITHMETIC_PLUS;
            second->sign = SF_ARITHMETIC_PLUS;

            SF_ARITHMETIC_SIGN_T rc = __sfbigint_add__(first, second, res);
            res->sign = SF_ARITHMETIC_MINUS;

            return rc;
        }
    }
}

#endif  // SF_ARITHMETIC_C
