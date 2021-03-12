#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/GrammarBuilder.h"

#include "ResourceHelpers.h"

using namespace testing;
using namespace parser;

TEST(GrammarBuilder, buildsExpressionGrammar) {
    GrammarBuilder builder;

    builder.defineRule("<expr>", {"<term>", "+", "<expr>"});
    builder.defineRule("<expr>", {"<term>"});

    builder.defineRule("<term>", {"<factor>", "*", "<term>"});
    builder.defineRule("<term>", {"<factor>"});

    builder.defineRule("<factor>", {"(", "<expr>", ")"});
    builder.defineRule("<factor>", {"<operand>"});

    builder.defineRule("<operand>", {"identifier"});
    builder.defineRule("<operand>", {"constant"});

    builder.build();

	//EXPECT_THAT(first0, SizeIs(4));
}
