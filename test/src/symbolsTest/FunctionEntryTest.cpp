#include "gtest/gtest.h"

#include "symbols/FunctionEntry.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

namespace {

TEST(FunctionEntry, storesNameTypeAndArguments) {
    translation_unit::Context ctx { "t.c", 1 };
    type::Type fnType = type::function(type::signedInteger(), { type::signedInteger() });
    symbols::FunctionEntry entry { "add", fnType.getFunction(), ctx };
    EXPECT_EQ(entry.getName(), "add");
    EXPECT_EQ(entry.argumentCount(), 1u);
    EXPECT_TRUE(entry.returnType().isPrimitive());
    EXPECT_EQ(entry.returnType().getSize(), 4);
}

} // namespace
