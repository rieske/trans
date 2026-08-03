#include "gtest/gtest.h"

#include "codegen/Sse.h"
#include "codegen/Value.h"

#include <stdexcept>

namespace {

using namespace codegen;

TEST(Sse, binMnemonicMatchesWidthAndOpOrder) {
    EXPECT_STREQ(sseBinMnemonic(SseBin::Add, SseWidth::F32), "addss");
    EXPECT_STREQ(sseBinMnemonic(SseBin::Sub, SseWidth::F32), "subss");
    EXPECT_STREQ(sseBinMnemonic(SseBin::Mul, SseWidth::F32), "mulss");
    EXPECT_STREQ(sseBinMnemonic(SseBin::Div, SseWidth::F32), "divss");
    EXPECT_STREQ(sseBinMnemonic(SseBin::Add, SseWidth::F64), "addsd");
    EXPECT_STREQ(sseBinMnemonic(SseBin::Div, SseWidth::F64), "divsd");
}

TEST(Sse, cvtFloatWidensOnlyF32ToF64) {
    EXPECT_TRUE(sseCvtFloatWidens(SseWidth::F32, SseWidth::F64));
    EXPECT_FALSE(sseCvtFloatWidens(SseWidth::F64, SseWidth::F32));
    EXPECT_THROW(sseCvtFloatWidens(SseWidth::F32, SseWidth::F32), std::runtime_error);
    EXPECT_THROW(sseCvtFloatWidens(SseWidth::F64, SseWidth::F64), std::runtime_error);
}

TEST(Sse, widthRequiresSseFloat) {
    Value f32 { "f", 0, ValueKind::FLOATING, 4 };
    Value f64 { "d", 1, ValueKind::FLOATING, 8 };
    Value ix { "i", 2, ValueKind::INTEGRAL, 8 };
    Value ld { "ld", 3, ValueKind::FLOATING, 16 };
    EXPECT_EQ(sseWidth(f32), SseWidth::F32);
    EXPECT_EQ(sseWidth(f64), SseWidth::F64);
    EXPECT_THROW(sseWidth(ix), std::runtime_error);
    EXPECT_THROW(sseWidth(ld), std::runtime_error);
}

} // namespace
