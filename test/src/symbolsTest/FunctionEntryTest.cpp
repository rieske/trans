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
    EXPECT_EQ(entry.argumentCount(), entry.arguments().size());
    EXPECT_EQ(entry.argumentCount(), entry.getType().argumentCount());
    EXPECT_EQ(entry.getContext().getSourceName(), "t.c");
    EXPECT_EQ(entry.getContext().getOffset(), 7u);
    EXPECT_TRUE(entry.returnType().isPrimitive());
    EXPECT_EQ(entry.returnType().getSize(), 4);
    auto args = entry.arguments();
    ASSERT_EQ(args.size(), 1u);
    EXPECT_TRUE(args[0].isPrimitive());
    EXPECT_EQ(args[0].getSize(), 4);
    EXPECT_TRUE(entry.getType().getReturnType().isPrimitive());
    EXPECT_FALSE(entry.hasInternalLinkage());
}

TEST(FunctionEntry, argumentsAndReturnAliasFunctionPayload) {
    translation_unit::Context ctx { "t.c", 1 };
    type::Type fnType = type::function(type::signedInteger(), { type::signedLong() });
    symbols::FunctionEntry entry { "f", fnType.getFunction(), ctx };
    EXPECT_EQ(&entry.arguments(), &entry.getType().getArguments());
    EXPECT_EQ(&entry.returnType(), &entry.getType().getReturnType());
}

TEST(FunctionEntry, recordsInternalLinkage) {
    translation_unit::Context ctx { "t.c", 1 };
    type::Type fnType = type::function(type::signedInteger(), {});
    symbols::FunctionEntry hidden { "hidden", fnType.getFunction(), ctx, true };
    EXPECT_TRUE(hidden.hasInternalLinkage());
}

TEST(FunctionEntry, zeroAndMultiArgArgumentCount) {
    translation_unit::Context ctx { "t.c", 1 };
    type::Type zero = type::function(type::voidType(), {});
    symbols::FunctionEntry noArgs { "f0", zero.getFunction(), ctx };
    EXPECT_EQ(noArgs.argumentCount(), 0u);
    EXPECT_EQ(noArgs.argumentCount(), noArgs.arguments().size());
    EXPECT_EQ(noArgs.argumentCount(), noArgs.getType().argumentCount());

    type::Type multi = type::function(type::signedInteger(),
            { type::signedInteger(), type::signedLong(), type::signedCharacter() });
    symbols::FunctionEntry three { "f3", multi.getFunction(), ctx };
    EXPECT_EQ(three.argumentCount(), 3u);
    EXPECT_EQ(three.argumentCount(), three.arguments().size());
    EXPECT_EQ(three.argumentCount(), three.getType().argumentCount());
}

} // namespace
