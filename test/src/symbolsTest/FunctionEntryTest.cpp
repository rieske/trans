#include "gtest/gtest.h"

#include <stdexcept>

#include "symbols/FunctionEntry.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

namespace {

TEST(FunctionEntry, storesNameTypeArgumentsAndContext) {
    translation_unit::Context ctx { "t.c", 7 };
    type::Type fnType = type::function(type::signedInteger(), { type::signedInteger() });
    symbols::FunctionEntry entry { "add", fnType, ctx };
    EXPECT_EQ(entry.getName(), "add");
    EXPECT_EQ(entry.arguments().size(), 1u);
    EXPECT_EQ(&entry.arguments(), &entry.getType().getFunction().getArguments());
    EXPECT_EQ(entry.getContext().getSourceName(), "t.c");
    EXPECT_EQ(entry.getContext().getOffset(), 7u);
    EXPECT_TRUE(entry.returnType().isPrimitive());
    EXPECT_EQ(entry.returnType().getSize(), 4);
    auto args = entry.arguments();
    ASSERT_EQ(args.size(), 1u);
    EXPECT_TRUE(args[0].isPrimitive());
    EXPECT_EQ(args[0].getSize(), 4);
    EXPECT_TRUE(entry.getType().isFunction());
    EXPECT_TRUE(entry.getType().getFunction().getReturnType().isPrimitive());
    EXPECT_FALSE(entry.hasInternalLinkage());
}

TEST(FunctionEntry, argumentsAndReturnAliasFunctionPayload) {
    translation_unit::Context ctx { "t.c", 1 };
    type::Type fnType = type::function(type::signedInteger(), { type::signedLong() });
    symbols::FunctionEntry entry { "f", fnType, ctx };
    EXPECT_EQ(&entry.arguments(), &entry.getType().getFunction().getArguments());
    EXPECT_EQ(&entry.returnType(), &entry.getType().getFunction().getReturnType());
}

TEST(FunctionEntry, rejectsNonFunctionType) {
    translation_unit::Context ctx { "t.c", 1 };
    EXPECT_THROW((symbols::FunctionEntry { "x", type::signedInteger(), ctx }), std::logic_error);
}

TEST(FunctionEntry, recordsInternalLinkage) {
    translation_unit::Context ctx { "t.c", 1 };
    type::Type fnType = type::function(type::signedInteger(), {});
    symbols::FunctionEntry hidden { "hidden", fnType, ctx, true };
    EXPECT_TRUE(hidden.hasInternalLinkage());
}

TEST(FunctionEntry, zeroAndMultiArgCounts) {
    translation_unit::Context ctx { "t.c", 1 };
    type::Type zero = type::function(type::voidType(), {});
    symbols::FunctionEntry noArgs { "f0", zero, ctx };
    EXPECT_EQ(noArgs.arguments().size(), 0u);

    type::Type multi = type::function(type::signedInteger(),
            { type::signedInteger(), type::signedLong(), type::signedCharacter() });
    symbols::FunctionEntry three { "f3", multi, ctx };
    EXPECT_EQ(three.arguments().size(), 3u);
}

} // namespace
