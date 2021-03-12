#include "ResourceHelpers.h"

#include "parser/BNFFileGrammar.h"
#include "parser/Grammar.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace testing;
using namespace parser;

TEST(BNFGrammarReader, readsBNFGrammarConfiguration) {
    BNFFileGrammar reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));

    EXPECT_THAT(grammar.ruleCount(), Eq(240));

    EXPECT_THAT(grammar.getTerminals(), SizeIs(86));
    for (const auto& terminal : grammar.getTerminals()) {
        EXPECT_THAT(terminal.isTerminal(), Eq(true));
        EXPECT_THAT(terminal.isNonterminal(), Eq(false));
    }
    EXPECT_THAT(grammar.getNonterminals(), SizeIs(67));
    for (const auto& nonterminal : grammar.getNonterminals()) {
        EXPECT_THAT(nonterminal.isNonterminal(), Eq(true));
        EXPECT_THAT(nonterminal.isTerminal(), Eq(false));
    }
}

TEST(BNFGrammarReader, readsExpressionGrammarBNF) {
    BNFFileGrammar reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/expression_grammar.bnf"));

    EXPECT_THAT(grammar.ruleCount(), Eq(9));

    EXPECT_THAT(grammar.getTerminals(), SizeIs(7));
    for (const auto& terminal : grammar.getTerminals()) {
        EXPECT_THAT(terminal.isTerminal(), Eq(true));
        EXPECT_THAT(terminal.isNonterminal(), Eq(false));
    }
    EXPECT_THAT(grammar.getNonterminals(), SizeIs(4));
    for (const auto& nonterminal : grammar.getNonterminals()) {
        EXPECT_THAT(nonterminal.isNonterminal(), Eq(true));
        EXPECT_THAT(nonterminal.isTerminal(), Eq(false));
    }

    EXPECT_THAT(grammar.getRuleByIndex(0).begin()->getRuleIndexes().size(), Eq(2));

    auto startProductions = grammar.getProductionsOfSymbol(grammar.getStartSymbol());
    EXPECT_THAT(startProductions, SizeIs(0));

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

TEST(BNFGrammarReader, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
    BNFFileGrammar reader;
    ASSERT_THROW(reader.readGrammar("nonexistentFile.abc"), std::invalid_argument);
}
