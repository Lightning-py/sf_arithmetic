#ifndef SF_ARITHMETIC_H
#define SF_ARITHMETIC_H

// для типов данных
#include <limits.h>  // limits for types
#include <stddef.h>  // types for digits and bigdigits
#include <stdint.h>  // size_t
#include <string.h>  // memcpy

// переобозначения типов для структуры
typedef int SF_ARITHMETIC_STATUS_CODE;  // тип хранения статус кодов
typedef uint32_t SF_ARITHMETIC_DIGITS_T;
typedef uint64_t SF_ARITHMETIC_BIGDIGITS_T;
typedef size_t SF_ARITHMETIC_SIZE_T;
typedef int SF_ARITHMETIC_SIGN_T;
typedef char SF_ARITHMETIC_CHAR_T;

#define SF_ARITHMETIC_DIGITS_T_SIZE (sizeof(SF_ARITHMETIC_DIGITS_T))
#define SF_ARITHMETIC_BIGDIGITS_T_SIZE (sizeof(SF_ARITHMETIC_BIGDIGITS_T))
#define SF_ARITHMETIC_CHAR_T_SIZE (sizeof(SF_ARITHMETIC_CHAR_T))

#define SF_ARITHMETIC_DIGITS_T_SIZE_BITS (SF_ARITHMETIC_DIGITS_T_SIZE * 8)
#define SF_ARITHMETIC_BIGDIGITS_T_SIZE_BITS (SF_ARITHMETIC_BIGDIGITS_T_SIZE * 8)
#define SF_ARITHMETIC_CHAR_T_SIZE_BITS (sizeof(SF_ARITHMETIC_CHAR_T) * 8)

#define SF_ARITHMETIC_DIGITS_T_FULL_MASK UINT32_MAX
#define SF_ARITHMETIC_BIGDIGITS_T_FULL_MASK UINT64_MAX
#define SF_ARITHMETIC_BIGDIGITS_LEFTHALF_MASK \
    (SF_ARITHMETIC_BIGDIGITS_T_FULL_MASK ^ SF_ARITHMETIC_DIGITS_T_FULL_MASK)

// дефайны для знаков числа
#define SF_ARITHMETIC_PLUS 0
#define SF_ARITHMETIC_MINUS 1

// дефайны для статус кодов

#define SF_ARITHMETIC_FINE 0           // все в порядке
#define SF_ARITHMETIC_MEMORY_ERROR -1  // ошибка памяти
#define SF_ARITHMETIC_INVALID_ARGUMENT \
    -2  // неправильный аргумент, невозможность выполнения операции с данным
        // аргументом

#define SF_ARITHMETIC_TRUE 1
#define SF_ARITHMETIC_FALSE 0

/*
основная структура большого числа
digits - массив с числами в системе счисления 2^32
size - количество чисел в массиве
sign - знак числа

назовем цифрой (с овнованием системы счисления 2^32) число в массиве digits

порядок битов в цифре определяется целевой системой
цифры в массиве хранятся от младших к старшим слева направо
*/

typedef struct {
    SF_ARITHMETIC_DIGITS_T* digits;
    SF_ARITHMETIC_SIZE_T size;
    SF_ARITHMETIC_SIGN_T sign;

} sfbigint_t;

// функция для проверка валидности объекта
SF_ARITHMETIC_STATUS_CODE
sfbigint_fine(sfbigint_t* obj);

// функции выделения и удаления объекта в памяти
SF_ARITHMETIC_STATUS_CODE
sfbigint_create(sfbigint_t** obj, SF_ARITHMETIC_DIGITS_T size);
SF_ARITHMETIC_STATUS_CODE sfbigint_free(sfbigint_t* obj);

SF_ARITHMETIC_STATUS_CODE sfbigint_normalise(sfbigint_t* obj);
SF_ARITHMETIC_STATUS_CODE sfbigint_setzero(sfbigint_t* obj);

SF_ARITHMETIC_STATUS_CODE sfbigint_copy(sfbigint_t* obj, sfbigint_t** copy);
SF_ARITHMETIC_STATUS_CODE sfbigint_swap(sfbigint_t* first, sfbigint_t* second);

SF_ARITHMETIC_STATUS_CODE sfbigint_iszero(sfbigint_t* obj);

/*
функция для проверки какое из чисел больше
результат 0 - числа равны
результат 1 - первое число больше
результат 2 - второе число больше
*/
SF_ARITHMETIC_SIGN_T sfbigint_compare(sfbigint_t* first, sfbigint_t* second);

SF_ARITHMETIC_STATUS_CODE sfbigint_lshift(sfbigint_t* obj,
                                          SF_ARITHMETIC_SIZE_T times);

SF_ARITHMETIC_STATUS_CODE sfbigint_rshift(sfbigint_t* obj,
                                          SF_ARITHMETIC_SIZE_T times);

SF_ARITHMETIC_STATUS_CODE sfbigint_divmod10(sfbigint_t* obj,
                                            SF_ARITHMETIC_DIGITS_T* res);

SF_ARITHMETIC_STATUS_CODE sfbigint_mul10(sfbigint_t* obj);

SF_ARITHMETIC_STATUS_CODE sfbigint_strtosfbigint(sfbigint_t** obj,
                                                 SF_ARITHMETIC_CHAR_T* str,
                                                 SF_ARITHMETIC_SIZE_T len);
SF_ARITHMETIC_STATUS_CODE sfbigint_tostring(sfbigint_t* obj,
                                            SF_ARITHMETIC_CHAR_T** str);

// стандартные арифметические функции
SF_ARITHMETIC_STATUS_CODE
sfbigint_add(sfbigint_t* first, sfbigint_t* second, sfbigint_t* res);
SF_ARITHMETIC_STATUS_CODE sfbigint_sub(sfbigint_t* first, sfbigint_t* second,
                                       sfbigint_t* res);
SF_ARITHMETIC_STATUS_CODE sfbigint_mul(sfbigint_t* first, sfbigint_t* second,
                                       sfbigint_t* res);
SF_ARITHMETIC_STATUS_CODE sfbigint_div(sfbigint_t* first, sfbigint_t* second,
                                       sfbigint_t* res);

//
SF_ARITHMETIC_STATUS_CODE
__sfbigint_add__(sfbigint_t* first, sfbigint_t* second, sfbigint_t* res);
SF_ARITHMETIC_STATUS_CODE __sfbigint_sub__(sfbigint_t* first,
                                           sfbigint_t* second, sfbigint_t* res);

/*
дефайны для определения способа выделения памяти
функции принимают такие же параметры как функции malloc, calloc и free из
стандартной библиотекой C
*/

#include <stdlib.h>  // malloc, calloc

#define SF_ARITHMETIC_MALLOC malloc
#define SF_ARITHMETIC_CALLOC calloc
#define SF_ARITHMETIC_REALLOC realloc
#define SF_ARITHMETIC_FREE free

#define SF_MAX(first, second) (first > second ? first : second)

#endif  // SF_ARITHMETIC_H
