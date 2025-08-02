#ifndef SF_ARITHMETIC_H
#define SF_ARITHMETIC_H

// для типов данных
#include <stddef.h> // uint32_t, uint64_t
#include <stdint.h> // size_t
#include <string.h> // memcpy

// дефайны для знаков числа
#define SF_ARITHMETIC_PLUS 0
#define SF_ARITHMETIC_MINUS 1

// дефайны для статус кодов
#define SF_ARITHMETIC_STATUS_CODE int // тип хранения статус кодов
#define SF_ARITHMETIC_FINE 0 // все в порядке
#define SF_ARITHMETIC_MEMORY_ERROR -1 // ошибка памяти
#define SF_ARITHMETIC_INVALID_ARGUMENT -2 // неправильный аргумент, невозможность выполнения операции с данным аргументом

// переобозначения типов для структуры
typedef uint32_t SF_ARITHMETIC_DIGITS_T; 
typedef uint64_t SF_ARITHMETIC_BIGDIGITS_T;
typedef size_t SF_ARITHMETIC_SIZE_T;
typedef int SF_ARITHMETIC_SIGN_T;

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
  SF_ARITHMETIC_DIGITS_T *digits;
  SF_ARITHMETIC_SIZE_T size;
  SF_ARITHMETIC_SIGN_T sign;
} sfbigint_t;

// функция для проверка валидности объекта
SF_ARITHMETIC_STATUS_CODE sfbigint_fine(sfbigint_t* obj);


// функции выделения и удаления объекта в памяти
SF_ARITHMETIC_STATUS_CODE sfbigint_create(sfbigint_t **obj, SF_ARITHMETIC_DIGITS_T size);
SF_ARITHMETIC_STATUS_CODE sfbigint_free(sfbigint_t *obj);

SF_ARITHMETIC_STATUS_CODE sfbigint_normalise(sfbigint_t* obj);
SF_ARITHMETIC_STATUS_CODE sfbigint_setzero(sfbigint_t* obj);

SF_ARITHMETIC_STATUS_CODE sfbigint_copy(sfbigint_t* obj, sfbigint_t** copy);
SF_ARITHMETIC_STATUS_CODE sfbigint_swap(sfbigint_t* first, sfbigint_t* second);

/*
функция для проверки какое из чисел больше
результат 0 - числа равны
результат 1 - первое число больше
результат 2 - второе число больше
*/
SF_ARITHMETIC_SIGN_T sfbigint_compare(sfbigint_t* first, sfbigint_t* second);

// стандартные арифметические функции
SF_ARITHMETIC_STATUS_CODE sfbigint_add(sfbigint_t *first, sfbigint_t *second, sfbigint_t *res);
SF_ARITHMETIC_STATUS_CODE sfbigint_sub(sfbigint_t *first, sfbigint_t *second, sfbigint_t *res);
SF_ARITHMETIC_STATUS_CODE sfbigint_mul(sfbigint_t *first, sfbigint_t *second, sfbigint_t *res);
SF_ARITHMETIC_STATUS_CODE sfbigint_div(sfbigint_t *first, sfbigint_t *second, sfbigint_t *res);

// 
SF_ARITHMETIC_STATUS_CODE sfbigint_add_(sfbigint_t* first, sfbigint_t* second, sfbigint_t* res);
SF_ARITHMETIC_STATUS_CODE sfbigint_sub_(sfbigint_t* first, sfbigint_t* second, sfbigint_t* res);



/*
дефайны для определения способа выделения памяти
функции принимают такие же параметры как функции malloc, calloc и free из стандартной библиотекой C
*/


#include <stdlib.h> // malloc, calloc

#define SF_ARITHMETIC_MALLOC malloc
#define SF_ARITHMETIC_CALLOC calloc
#define SF_ARITHMETIC_REALLOC realloc
#define SF_ARITHMETIC_FREE free

#define SF_MAX(first, second) ( first > second ? first : second )

#endif // SF_ARITHMETIC_H