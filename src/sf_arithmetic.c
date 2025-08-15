#ifndef SF_ARITHMETIC_C
#define SF_ARITHMETIC_C

#include "sf_arithmetic.h"

SF_ARITHMETIC_STATUS_CODE sfbigint_fine(sfbigint_t* obj) {
    return (obj && obj->digits && obj->size > 0 && (obj->sign == SF_ARITHMETIC_PLUS || obj->sign == SF_ARITHMETIC_MINUS)) ? SF_ARITHMETIC_FINE
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
    (*copy)->sign = obj->sign;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_move(sfbigint_t* obj, sfbigint_t** copy) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    *copy = (sfbigint_t*) SF_ARITHMETIC_MALLOC(sizeof(sfbigint_t));

    (*copy)->digits = obj->digits;

    (*copy)->size = obj->size;
    (*copy)->sign = obj->sign;

    SF_ARITHMETIC_FREE(obj);

    return SF_ARITHMETIC_FINE;
}



SF_ARITHMETIC_STATUS_CODE sfbigint_copytoexistent(sfbigint_t* obj, sfbigint_t* copy) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE || sfbigint_fine(copy) != SF_ARITHMETIC_FINE) return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_FREE(copy->digits);
    
    SF_ARITHMETIC_DIGITS_T* new_digits = SF_ARITHMETIC_CALLOC(obj->size, SF_ARITHMETIC_DIGITS_T_SIZE);
    if (!new_digits) return SF_ARITHMETIC_MEMORY_ERROR;
    
    memcpy(new_digits, obj->digits, obj->size * SF_ARITHMETIC_DIGITS_T_SIZE);

    copy->digits = new_digits;
    copy->size = obj->size;
    copy->sign = obj->sign;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_movetoexistent(sfbigint_t* obj, sfbigint_t* copy) {
    if (sfbigint_fine(obj) != SF_ARITHMETIC_FINE || sfbigint_fine(copy) != SF_ARITHMETIC_FINE) return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_FREE(copy->digits);
    
    copy->digits = obj->digits;
    copy->size = obj->size;
    copy->sign = obj->sign;

    obj->digits = NULL;
    SF_ARITHMETIC_FREE(obj);

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

    for (SF_ARITHMETIC_SIZE_T i = new_size - 1; i < obj->size && obj->digits[i] == 0; i--)
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
    obj->size = new_size;

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
    for (SF_ARITHMETIC_SIZE_T i = obj->size - 1; i < obj->size ; i--) {
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
                                                 const SF_ARITHMETIC_CHAR_T* str,
                                                 SF_ARITHMETIC_SIZE_T len) {
    sfbigint_t* obj_ = NULL;
    SF_ARITHMETIC_STATUS_CODE rc_create = sfbigint_create(&obj_, 1);

    if (rc_create != SF_ARITHMETIC_FINE) return rc_create;

    SF_ARITHMETIC_SIZE_T i = 0;

    if (str[0] == '-') {
        i++;
        obj_->sign = SF_ARITHMETIC_MINUS;
    }

    for (; i < len; ++i) {
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
    
    SF_ARITHMETIC_SIZE_T num_count = 0;
    
    if (sfbigint_iszero(obj) == SF_ARITHMETIC_TRUE) {
        str[0] = '0';
        goto end;
    }

    SF_ARITHMETIC_STATUS_CODE rc_copy = sfbigint_copy(obj, &obj_);
    if (rc_copy != SF_ARITHMETIC_FINE) goto err;

    while (!sfbigint_iszero(obj_)) {
        SF_ARITHMETIC_DIGITS_T num = 0;
        SF_ARITHMETIC_STATUS_CODE rc_divmod = sfbigint_divmod10(obj_, &num);

        if (rc_divmod != SF_ARITHMETIC_FINE) goto err_after_obj;

        if (num_count + 2 >= str_size) {
            str_size *= 2;
            SF_ARITHMETIC_CHAR_T* temp = SF_ARITHMETIC_REALLOC(
                str, sizeof(SF_ARITHMETIC_CHAR_T) * str_size);

            if (!temp) goto err_after_obj;

            str = temp;
        }

        str[num_count] = '0' + num;
        num_count++;
    }

    if (obj_->sign == SF_ARITHMETIC_MINUS) {
        str[num_count] = '-';
        num_count++;
    }

    for (SF_ARITHMETIC_SIZE_T i = 0; i < num_count / 2; ++i) {
        SF_ARITHMETIC_CHAR_T temp = str[i];
        str[i] = str[num_count - i - 1];
        str[num_count - i - 1] = temp;
    }

    goto end;

err_after_obj:
    sfbigint_free(obj_);

err:
    SF_ARITHMETIC_FREE(str);

    return SF_ARITHMETIC_MEMORY_ERROR;

end:
    sfbigint_free(obj_);

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

// функция +=, прибавляет первое число ко второму и записывает результат в первое
SF_ARITHMETIC_STATUS_CODE sfbigint_addeq(sfbigint_t* first, sfbigint_t* second) {
    sfbigint_t* temp = NULL;
    
    SF_ARITHMETIC_STATUS_CODE rc_create = sfbigint_create(&temp, 1);
    if (rc_create != SF_ARITHMETIC_FINE) return rc_create;

    SF_ARITHMETIC_STATUS_CODE rc_add = sfbigint_add(first, second, temp);
    if (rc_add != SF_ARITHMETIC_FINE) return rc_add;

    SF_ARITHMETIC_FREE(first->digits);

    first->digits = temp->digits;
    first->size = temp->size;
    first->sign = temp->sign;

    SF_ARITHMETIC_FREE(temp);

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_subeq(sfbigint_t* first, sfbigint_t* second) {
    sfbigint_t* temp = NULL;
    SF_ARITHMETIC_STATUS_CODE rc_create = sfbigint_create(&temp, 1);

    if (rc_create != SF_ARITHMETIC_FINE) return rc_create;

    SF_ARITHMETIC_STATUS_CODE rc_sub = sfbigint_sub(first, second, temp);
    if (rc_sub != SF_ARITHMETIC_FINE) return rc_sub;

    SF_ARITHMETIC_FREE(first->digits);

    first->digits = temp->digits;
    first->size = temp->size;
    first->sign = temp->sign;

    SF_ARITHMETIC_FREE(temp);

    return SF_ARITHMETIC_FINE;
}

// Вспомогательная функция для разделения числа на две части
static SF_ARITHMETIC_STATUS_CODE sfbigint_split(sfbigint_t* num,
                                                SF_ARITHMETIC_SIZE_T m,
                                                sfbigint_t* low,
                                                sfbigint_t* high) {
    if (!num || !low || !high) return SF_ARITHMETIC_INVALID_ARGUMENT;

    // Инициализируем low часть (первые m цифр)
    low->size = m;
    low->digits = num->digits;
    low->sign = SF_ARITHMETIC_PLUS;

    // Инициализируем high часть (остальные цифры)
    high->size = num->size - m;
    high->digits = num->digits + m;
    high->sign = SF_ARITHMETIC_PLUS;

    return SF_ARITHMETIC_FINE;
}

SF_ARITHMETIC_STATUS_CODE sfbigint_mul_digits(sfbigint_t* obj, SF_ARITHMETIC_DIGITS_T num) {
    sfbigint_t* res = NULL;
    sfbigint_t* temp = NULL;

    SF_ARITHMETIC_STATUS_CODE rc_addeq = SF_ARITHMETIC_FINE;
    SF_ARITHMETIC_STATUS_CODE rc_shift = SF_ARITHMETIC_FINE;
    SF_ARITHMETIC_STATUS_CODE rc_create = SF_ARITHMETIC_FINE;

    rc_create = sfbigint_create(&temp, 2);
    if (rc_create != SF_ARITHMETIC_FINE) return rc_create;

    rc_create = sfbigint_create(&res, obj->size + 1);
    if (rc_create != SF_ARITHMETIC_FINE) return rc_create;

    temp->sign = SF_ARITHMETIC_PLUS;


    for (SF_ARITHMETIC_SIZE_T i = obj->size - 1; i < obj->size; --i) {
        SF_ARITHMETIC_BIGDIGITS_T temp_digits = (SF_ARITHMETIC_BIGDIGITS_T) obj->digits[i] * (SF_ARITHMETIC_BIGDIGITS_T) num;
        
        temp->digits[0] = temp_digits & SF_ARITHMETIC_DIGITS_T_FULL_MASK;
        temp->digits[1] = temp_digits & SF_ARITHMETIC_BIGDIGITS_LEFTHALF_MASK;

        rc_addeq = sfbigint_addeq(res, temp);
        if (rc_addeq != SF_ARITHMETIC_FINE) {
            sfbigint_free(res);
            sfbigint_free(temp);
            return rc_addeq;
        }

        if (i != 0) {
            rc_shift = sfbigint_lshift(res, SF_ARITHMETIC_DIGITS_T_SIZE_BITS);
            if (rc_shift != SF_ARITHMETIC_FINE) return rc_shift;
        }
    }

    sfbigint_free(temp);
    

    SF_ARITHMETIC_STATUS_CODE rc_moveext = sfbigint_movetoexistent(res, obj);
    if (rc_moveext != SF_ARITHMETIC_FINE) return rc_moveext;


    return SF_ARITHMETIC_FINE;
}

// // // Наивное умножение (используется для маленьких чисел)
SF_ARITHMETIC_STATUS_CODE sfbigint_mul_naive(sfbigint_t* first,
                                                    sfbigint_t* second,
                                                    sfbigint_t** res) {
    if (sfbigint_fine(first) != SF_ARITHMETIC_FINE ||
        sfbigint_fine(second) != SF_ARITHMETIC_FINE)
        return SF_ARITHMETIC_MEMORY_ERROR;

    SF_ARITHMETIC_STATUS_CODE rc = SF_ARITHMETIC_FINE;

    sfbigint_t* res_ = NULL;
    sfbigint_t* temp = NULL;
    
    rc = sfbigint_create(&res_, first->size * second->size);
    if (rc != SF_ARITHMETIC_FINE) return rc;


    for (SF_ARITHMETIC_SIZE_T i = 0; i < second->size; ++i) {
        if (i == 0)
            rc = sfbigint_copy(first, &temp); 
        else 
            rc = sfbigint_copytoexistent(first, temp);
        if (rc != SF_ARITHMETIC_FINE) goto err;
        
        rc = sfbigint_mul_digits(temp, second->digits[i]);
        if (rc != SF_ARITHMETIC_FINE) goto err;

        rc = sfbigint_lshift(temp, SF_ARITHMETIC_DIGITS_T_SIZE_BITS * i);
        if (rc != SF_ARITHMETIC_FINE) goto err;

        rc = sfbigint_addeq(res_, temp);

        if (rc != SF_ARITHMETIC_FINE) goto err;
    }   

    goto fine;

err:
    sfbigint_free(res_);
    sfbigint_free(temp);

    return SF_ARITHMETIC_MEMORY_ERROR;

fine:
    sfbigint_free(temp);


    rc = sfbigint_move(res_, res);
    if (rc != SF_ARITHMETIC_FINE) sfbigint_free(res_);

    return rc;
}


#endif  // SF_ARITHMETIC_C
