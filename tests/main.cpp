#include <gtest/gtest.h>
#include "sf_arithmetic.h"
// Тестируем C-функцию
extern "C" {
    #include "sf_arithmetic.h"  // Подключаем ещё раз для C-линковки
}

TEST(LOL_TEST, TestAddition) {
    EXPECT_EQ(2 + 3, 5); 
}

TEST(LOL_TEST, TestSubtraction) {
    EXPECT_EQ(2 - 3, -1);
}