#include "ResourceHelpers.h"

#include "parser/BNFFileReader.h"
#include "parser/Grammar.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace testing;
using namespace parser;

TEST(BNFGrammarReader, readsBNFGrammarConfiguration) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));

    EXPECT_THAT(grammar.ruleCount(), Eq(240));

    EXPECT_THAT(grammar.getTerminalIDs(), SizeIs(86));
    EXPECT_THAT(grammar.getNonterminalIDs(), SizeIs(67));
}

TEST(BNFGrammarReader, readsExpressionGrammarBNF) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/expression_grammar.bnf"));

    EXPECT_THAT(grammar.ruleCount(), Eq(9));

    EXPECT_THAT(grammar.getTerminalIDs(), SizeIs(7));
    EXPECT_THAT(grammar.getNonterminalIDs(), SizeIs(4));

    auto startProductions = grammar.getProductionsOfSymbol(grammar.getStartSymbol().getId());
    EXPECT_THAT(startProductions, SizeIs(1));

    auto exprProductions = grammar.getProductionsOfSymbol(grammar.symbolId("<expr>"));
    EXPECT_THAT(exprProductions, SizeIs(2));
    EXPECT_THAT(grammar.str(exprProductions.at(0)), Eq("<expr> -> <term> + <expr>"));
    EXPECT_THAT(grammar.str(exprProductions.at(1)), Eq("<expr> -> <term>"));

    auto termProductions = grammar.getProductionsOfSymbol(grammar.symbolId("<term>"));
    EXPECT_THAT(termProductions, SizeIs(2));
    EXPECT_THAT(grammar.str(termProductions.at(0)), Eq("<term> -> <factor> * <term>"));
    EXPECT_THAT(grammar.str(termProductions.at(1)), Eq("<term> -> <factor>"));

    auto factorProductions = grammar.getProductionsOfSymbol(grammar.symbolId("<factor>"));
    EXPECT_THAT(factorProductions, SizeIs(2));
    EXPECT_THAT(grammar.str(factorProductions.at(0)), Eq("<factor> -> ( <expr> )"));
    EXPECT_THAT(grammar.str(factorProductions.at(1)), Eq("<factor> -> <operand>"));

    auto operandProductions = grammar.getProductionsOfSymbol(grammar.symbolId("<operand>"));
    EXPECT_THAT(operandProductions, SizeIs(2));
    EXPECT_THAT(grammar.str(operandProductions.at(0)), Eq("<operand> -> identifier"));
    EXPECT_THAT(grammar.str(operandProductions.at(1)), Eq("<operand> -> constant"));
}

TEST(BNFGrammarReader, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
    BNFFileReader reader;
    ASSERT_THROW(reader.readGrammar("nonexistentFile.abc"), std::invalid_argument);
}
