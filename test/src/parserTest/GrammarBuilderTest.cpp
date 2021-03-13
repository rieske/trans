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

    Grammar grammar = builder.build();

	EXPECT_THAT(grammar.ruleCount(), Eq(9));
	EXPECT_THAT(grammar.getTerminals(), SizeIs(7));
	EXPECT_THAT(grammar.getNonterminals(), SizeIs(4));
	EXPECT_THAT(grammar.getNonterminalIDs(), SizeIs(4));

    auto startProductions = grammar.getProductionsOfSymbol(grammar.getStartSymbol().getId());
    EXPECT_THAT(startProductions, SizeIs(1));

    auto exprProductions = grammar.getProductionsOfSymbol("<expr>");
    EXPECT_THAT(exprProductions, SizeIs(2));
    EXPECT_THAT(grammar.str(exprProductions.at(0)), Eq("<expr> -> <term> + <expr>"));
    EXPECT_THAT(grammar.str(exprProductions.at(1)), Eq("<expr> -> <term>"));

    auto termProductions = grammar.getProductionsOfSymbol("<term>");
    EXPECT_THAT(termProductions, SizeIs(2));
    EXPECT_THAT(grammar.str(termProductions.at(0)), Eq("<term> -> <factor> * <term>"));
    EXPECT_THAT(grammar.str(termProductions.at(1)), Eq("<term> -> <factor>"));

    auto factorProductions = grammar.getProductionsOfSymbol("<factor>");
    EXPECT_THAT(factorProductions, SizeIs(2));
    EXPECT_THAT(grammar.str(factorProductions.at(0)), Eq("<factor> -> ( <expr> )"));
    EXPECT_THAT(grammar.str(factorProductions.at(1)), Eq("<factor> -> <operand>"));

    auto operandProductions = grammar.getProductionsOfSymbol("<operand>");
    EXPECT_THAT(operandProductions, SizeIs(2));
    EXPECT_THAT(grammar.str(operandProductions.at(0)), Eq("<operand> -> identifier"));
    EXPECT_THAT(grammar.str(operandProductions.at(1)), Eq("<operand> -> constant"));
}

