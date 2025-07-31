#ifndef SF_ARITHMETIC_H
#define SF_ARITHMETIC_H

// для типов данных
#include <stddef.h> // uint32_t, uint64_t
#include <stdint.h> // size_t

// дефайны для знаков числа
#define SF_ARITHMETIC_PLUS 0
#define SF_ARITHMETIC_MINUS 1

// дефайны для статус кодов
#define SF_ARITHMETIC_STATUS_CODE int // тип хранения статус кодов
#define SF_FINE 0 // все в порядке
#define SF_MEMORY_ERROR 1 // ошибка памяти

// переобозначения типов для структуры
typedef uint32_t SF_ARITHMETIC_DIGITS_T; 
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


// функции выделения и удаления объекта в памяти
SF_ARITHMETIC_STATUS_CODE sfbigint_create(sfbigint_t *obj, SF_ARITHMETIC_DIGITS_T);
SF_ARITHMETIC_STATUS_CODE sfbigint_free(sfbigint_t *obj);


// стандартные арифметические функции
sfbigint_t *sfbigint_add(sfbigint_t *first, sfbigint_t *second);
sfbigint_t *sfbigint_sub(sfbigint_t *first, sfbigint_t *second);
sfbigint_t *sfbigint_mul(sfbigint_t *first, sfbigint_t *second);
sfbigint_t *sfbigint_div(sfbigint_t *first, sfbigint_t *second);

/*
дефайны для определения способа выделения памяти
функции принимают такие же параметры как функции malloc, calloc и free из стандартной библиотекой C
*/


#include <stdlib.h> // malloc, calloc

#define SF_ARITHMETIC_MALLOC malloc
#define SF_ARITHMETIC_CALLOC calloc
#define SF_ARITHMETIC_FREE free

#endif // SF_ARITHMETIC_H