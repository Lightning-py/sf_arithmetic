#include <gtest/gtest.h>
#include <iostream>
// #include "sf_arithmetic.h"
// Тестируем C-функцию
extern "C" {
    #include "sf_arithmetic.h"  // Подключаем ещё раз для C-линковки
}


TEST(objects, create_free) {
    sfbigint_t* obj = NULL;
    SF_ARITHMETIC_SIGN_T rc = sfbigint_create(&obj, 1);
    
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);

    EXPECT_EQ(obj->digits[0], 0);

    sfbigint_free(obj);
}

TEST(normalisation, simpletest) {
    sfbigint_t* obj = NULL;

    SF_ARITHMETIC_SIGN_T rc = sfbigint_create(&obj, 2);
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);

    obj->digits[0] = 1;
    obj->digits[1] = 0;

    rc = sfbigint_normalise(obj);
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);

    EXPECT_EQ(obj->digits[0], 1);
    EXPECT_EQ(obj->size, 1);

    sfbigint_free(obj);
}

TEST(compare, bigger_by_size) {
    sfbigint_t* obj1 = NULL;
    sfbigint_t* obj2 = NULL;

    SF_ARITHMETIC_SIGN_T rc = sfbigint_create(&obj1, 2);
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);

    rc = sfbigint_create(&obj2, 1);
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);

    obj1->digits[1] = 1;
    obj2->digits[0] = 1;

    EXPECT_EQ(sfbigint_compare(obj1, obj2), 1);

    sfbigint_free(obj1);
    sfbigint_free(obj2);
}

TEST(addition, simpleadd) {
    sfbigint_t* obj1 = NULL;
    sfbigint_t* obj2 = NULL;
    sfbigint_t* res = NULL;

    SF_ARITHMETIC_SIGN_T rc = sfbigint_create(&obj1, 1);
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);

    rc = sfbigint_create(&obj2, 1);
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);

    rc = sfbigint_create(&res, 1);
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);

    obj1->digits[0] = 1;
    obj2->digits[0] = 2;

    rc = sfbigint_add_(obj1, obj2, res);
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);
    EXPECT_EQ(res->digits[0], 3);

    sfbigint_free(obj1);
    sfbigint_free(obj2);
    sfbigint_free(res);
}

TEST(addition, simpleadd2) {
    sfbigint_t* obj1 = NULL;
    sfbigint_t* obj2 = NULL;
    sfbigint_t* res = NULL;

    SF_ARITHMETIC_SIGN_T rc = sfbigint_create(&obj1, 1);
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);

    rc = sfbigint_create(&obj2, 1);
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);

    rc = sfbigint_create(&res, 1);
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);
    
    obj1->digits[0] = 1 << 31;
    obj2->digits[0] = 1 << 31;

    rc = sfbigint_add_(obj1, obj2, res);
    EXPECT_EQ(rc, SF_ARITHMETIC_FINE);

    EXPECT_EQ(res->digits[0], 0);
    EXPECT_EQ(res->digits[1], 1);

    sfbigint_free(obj1);
    sfbigint_free(obj2);
    sfbigint_free(res);
}

TEST(subtraction, first_bigger_by_size) {
    sfbigint_t* first = NULL;
    sfbigint_t* second = NULL;
    sfbigint_t* res = NULL;

    int rc_first = sfbigint_create(&first, 2);
    int rc_second = sfbigint_create(&second, 1);
    int rc_res = sfbigint_create(&res, 1);

    EXPECT_EQ(rc_first, SF_ARITHMETIC_FINE);
    EXPECT_EQ(rc_second, SF_ARITHMETIC_FINE);
    EXPECT_EQ(rc_res, SF_ARITHMETIC_FINE);

    first->digits[1] = 1;
    second->digits[0] = 1;

    int rc_subtraction = sfbigint_sub_(first, second, res);

    EXPECT_EQ(rc_subtraction, SF_ARITHMETIC_FINE);
    
    EXPECT_EQ(res->digits[0], (SF_ARITHMETIC_DIGITS_T) ( (SF_ARITHMETIC_BIGDIGITS_T) 1 << 32) - 1);
    EXPECT_EQ(res->sign, SF_ARITHMETIC_PLUS);

    sfbigint_free(first);
    sfbigint_free(second);
    sfbigint_free(res);
}

TEST(subtraction, second_bigger_by_size) {
    sfbigint_t* first = NULL;
    sfbigint_t* second = NULL;
    sfbigint_t* res = NULL;

    int rc_first = sfbigint_create(&first, 1);
    int rc_second = sfbigint_create(&second, 2);
    int rc_res = sfbigint_create(&res, 1);

    EXPECT_EQ(rc_first, SF_ARITHMETIC_FINE);
    EXPECT_EQ(rc_second, SF_ARITHMETIC_FINE);
    EXPECT_EQ(rc_res, SF_ARITHMETIC_FINE);

    first->digits[0] = 1;
    second->digits[1] = 1;

    int rc_subtraction = sfbigint_sub_(first, second, res);

    EXPECT_EQ(rc_subtraction, SF_ARITHMETIC_FINE);
    
    EXPECT_EQ(res->digits[0], (SF_ARITHMETIC_DIGITS_T) ( (SF_ARITHMETIC_BIGDIGITS_T) 1 << 32) - 1);
    EXPECT_EQ(res->sign, SF_ARITHMETIC_MINUS);

    sfbigint_free(first);
    sfbigint_free(second);
    sfbigint_free(res);
}

TEST(subtraction, first_bigger) {
    sfbigint_t* first = NULL;
    sfbigint_t* second = NULL;
    sfbigint_t* res = NULL;

    int rc_first = sfbigint_create(&first, 1);
    int rc_second = sfbigint_create(&second, 1);
    int rc_res = sfbigint_create(&res, 1);

    EXPECT_EQ(rc_first, SF_ARITHMETIC_FINE);
    EXPECT_EQ(rc_second, SF_ARITHMETIC_FINE);
    EXPECT_EQ(rc_res, SF_ARITHMETIC_FINE);
    
    first->digits[0] = 3;
    second->digits[0] = 1;

    int rc_subtraction = sfbigint_sub_(first, second, res);

    EXPECT_EQ(rc_subtraction, SF_ARITHMETIC_FINE);
    
    EXPECT_EQ(res->digits[0], 2);
    EXPECT_EQ(res->sign, SF_ARITHMETIC_PLUS);

    sfbigint_free(first);
    sfbigint_free(second);
    sfbigint_free(res);
}

TEST(subtraction, second_bigger) {
    sfbigint_t* first = NULL;
    sfbigint_t* second = NULL;
    sfbigint_t* res = NULL;

    int rc_first = sfbigint_create(&first, 1);
    int rc_second = sfbigint_create(&second, 1);
    int rc_res = sfbigint_create(&res, 1);

    EXPECT_EQ(rc_first, SF_ARITHMETIC_FINE);
    EXPECT_EQ(rc_second, SF_ARITHMETIC_FINE);
    EXPECT_EQ(rc_res, SF_ARITHMETIC_FINE);

    first->digits[0] = 1;
    second->digits[0] = 3;

    int rc_subtraction = sfbigint_sub_(first, second, res);
    
    EXPECT_EQ(rc_subtraction, SF_ARITHMETIC_FINE);

    EXPECT_EQ(res->digits[0], 2);
    EXPECT_EQ(res->sign, SF_ARITHMETIC_MINUS);

    sfbigint_free(first);
    sfbigint_free(second);
    sfbigint_free(res);
}