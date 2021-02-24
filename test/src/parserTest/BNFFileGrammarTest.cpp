#include "ResourceHelpers.h"

#include "parser/BNFFileGrammar.h"
#include "parser/Grammar.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace testing;
using namespace parser;

TEST(BNFFileGrammar, readsBNFGrammarConfiguration) {
    BNFFileGrammar grammar { getResourcePath("configuration/grammar.bnf") };

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

TEST(BNFFileGrammar, readsExpressionGrammarBNF) {
    BNFFileGrammar grammar { getTestResourcePath("grammars/expression_grammar.bnf") };

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

    auto exprProductions = grammar.getProductionsOfSymbol(grammar.getNonterminals().at(0));
    EXPECT_THAT(exprProductions, SizeIs(2));
    EXPECT_THAT(exprProductions.at(0).producedSequence(), ElementsAre("<term>", "+", "<expr>"));
    EXPECT_THAT(exprProductions.at(1).producedSequence(), ElementsAre("<term>"));
    EXPECT_THAT(exprProductions.at(0).begin()->getRuleIndexes().size(), Eq(2));
    EXPECT_THAT(exprProductions.at(1).begin()->getRuleIndexes().size(), Eq(2));

    auto termProductions = grammar.getProductionsOfSymbol(grammar.getNonterminals().at(1));
    EXPECT_THAT(termProductions, SizeIs(2));
    EXPECT_THAT(termProductions.at(0).producedSequence(), ElementsAre("<factor>", "*", "<term>"));
    EXPECT_THAT(termProductions.at(1).producedSequence(), ElementsAre("<factor>"));
    EXPECT_THAT(termProductions.at(0).begin()->getRuleIndexes().size(), Eq(2));
    EXPECT_THAT(termProductions.at(1).begin()->getRuleIndexes().size(), Eq(2));

    auto factorProductions = grammar.getProductionsOfSymbol(grammar.getNonterminals().at(2));
    EXPECT_THAT(factorProductions, SizeIs(2));
    EXPECT_THAT(factorProductions.at(0).producedSequence(), ElementsAre("(", "<expr>", ")"));
    EXPECT_THAT(factorProductions.at(1).producedSequence(), ElementsAre("<operand>"));
    EXPECT_THAT(factorProductions.at(1).begin()->getRuleIndexes().size(), Eq(2));

    auto operandrProductions = grammar.getProductionsOfSymbol(grammar.getNonterminals().at(3));
    EXPECT_THAT(operandrProductions, SizeIs(2));
    EXPECT_THAT(operandrProductions.at(0).producedSequence(), ElementsAre("identifier"));
    EXPECT_THAT(operandrProductions.at(1).producedSequence(), ElementsAre("constant"));
    EXPECT_THAT(operandrProductions.at(0).begin()->getRuleIndexes().size(), Eq(0));
    EXPECT_THAT(operandrProductions.at(1).begin()->getRuleIndexes().size(), Eq(0));
}

TEST(BNFFileGrammar, throwsInvalidArgumentWhenNotAbleToReadConfiguration) {
    ASSERT_THROW(BNFFileGrammar { "nonexistentFile.abc" }, std::invalid_argument);
}
