#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/InternalError.h"
#include "codegen/Address.h"
#include "codegen/Value.h"

#include <stdexcept>

namespace {

TEST(InternalError, requireRejectsAMissingValue) {
    EXPECT_THAT([] { codegen::require(nullptr, "IndexPlan for array codegen"); },
            testing::ThrowsMessage<std::logic_error>(
                    testing::AllOf(testing::HasSubstr("internal compiler error"),
                            testing::HasSubstr("IndexPlan for array codegen"))));
}

TEST(Value, getAssignedRegisterWithoutARegisterIsAnInternalError) {
    const codegen::Value value { 1, 0, codegen::Type::INTEGRAL, 4 };

    ASSERT_TRUE(value.isStored());
    EXPECT_THROW(value.getAssignedRegister(), std::logic_error);
}

TEST(Address, frameAccessorsRejectAGlobalHome) {
    const codegen::Address global = codegen::Address::globalLabel("g", 4);

    EXPECT_THROW(global.frameBase(), std::logic_error);
    EXPECT_THROW(global.offsetBytes(), std::logic_error);
}

TEST(Address, labelRejectsAFrameHome) {
    const codegen::Address local = codegen::Address::frame(codegen::FrameBase::BasePointer, -8, 4);

    EXPECT_THROW(local.label(), std::logic_error);
}

} // namespace
