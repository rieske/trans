// Direct tests of runtime/va_tls.c (linked as build/va_tls.o).
#include "gtest/gtest.h"
#include "gmock/gmock.h"

extern "C" {
void __trans_va_set_areas(void* reg_save, void* overflow);
void __trans_va_pop_areas(void);
void* __trans_va_get_reg_save(void);
void* __trans_va_get_overflow(void);
}

using namespace testing;

namespace {

TEST(VaTlsRuntime, emptyStackReturnsNull) {
    // Ensure we start from a clean depth (pop any leftover).
    for (int i = 0; i < 8; ++i) {
        __trans_va_pop_areas();
    }
    EXPECT_THAT(__trans_va_get_reg_save(), IsNull());
    EXPECT_THAT(__trans_va_get_overflow(), IsNull());
}

TEST(VaTlsRuntime, pushPopRestoresOuterFrame) {
    for (int i = 0; i < 8; ++i) {
        __trans_va_pop_areas();
    }
    int outerReg = 1;
    int outerOver = 2;
    int innerReg = 3;
    int innerOver = 4;

    __trans_va_set_areas(&outerReg, &outerOver);
    EXPECT_THAT(__trans_va_get_reg_save(), Eq(static_cast<void*>(&outerReg)));
    EXPECT_THAT(__trans_va_get_overflow(), Eq(static_cast<void*>(&outerOver)));

    __trans_va_set_areas(&innerReg, &innerOver);
    EXPECT_THAT(__trans_va_get_reg_save(), Eq(static_cast<void*>(&innerReg)));
    EXPECT_THAT(__trans_va_get_overflow(), Eq(static_cast<void*>(&innerOver)));

    __trans_va_pop_areas();
    EXPECT_THAT(__trans_va_get_reg_save(), Eq(static_cast<void*>(&outerReg)));
    EXPECT_THAT(__trans_va_get_overflow(), Eq(static_cast<void*>(&outerOver)));

    __trans_va_pop_areas();
    EXPECT_THAT(__trans_va_get_reg_save(), IsNull());
    EXPECT_THAT(__trans_va_get_overflow(), IsNull());
}

TEST(VaTlsRuntime, popOnEmptyIsSafe) {
    for (int i = 0; i < 8; ++i) {
        __trans_va_pop_areas();
    }
    __trans_va_pop_areas();
    EXPECT_THAT(__trans_va_get_reg_save(), IsNull());
}

} // namespace
