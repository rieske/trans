#include "gtest/gtest.h"

#include "symbols/FunctionEntry.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

namespace {

TEST(FunctionEntry, storesNameTypeArgumentsAndContext) {
    translation_unit::Context ctx { "t.c", 7 };
    type::Type fnType = type::function(type::signedInteger(), { type::signedInteger() });
    symbols::FunctionEntry entry { "add", fnType.getFunction(), ctx };
    EXPECT_EQ(entry.getName(), "add");
    EXPECT_EQ(entry.argumentCount(), 1u);
    EXPECT_EQ(entry.getContext().getSourceName(), "t.c");
    EXPECT_EQ(entry.getContext().getOffset(), 7u);
    EXPECT_TRUE(entry.returnType().isPrimitive());
    EXPECT_EQ(entry.returnType().getSize(), 4);
    auto args = entry.arguments();
    ASSERT_EQ(args.size(), 1u);
    EXPECT_TRUE(args[0].isPrimitive());
    EXPECT_EQ(args[0].getSize(), 4);
    EXPECT_TRUE(entry.getType().getReturnType().isPrimitive());
}

} // namespace
