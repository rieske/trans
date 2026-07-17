#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileReader.h"
#include "parser/LR1Item.h"
#include "parser/Production.h"

#include "ResourceHelpers.h"

#include <stdexcept>

using namespace testing;
using namespace parser;

TEST(LR1Item, constructsItemFromGrammarRuleAndLookahead) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/expression_grammar.bnf"));
    // Use a real terminal from the grammar as lookahead
    const int la = grammar.symbolId("identifier");
    Production production { 1, { 2, 3, 4 }, 0 };
    LR1Item item { production, std::vector<int>{ la }, grammar };

    EXPECT_THAT(item.getProduction().getDefiningSymbol(), Eq(1));
    EXPECT_THAT(item.getVisited(), SizeIs(0));
    EXPECT_THAT(item.getExpectedSymbols(), SizeIs(3));
    EXPECT_THAT(item.getLookaheads(grammar), ElementsAre(la));
}

TEST(LR1Item, advancesTheVisitedSymbols) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/expression_grammar.bnf"));
    const int la = grammar.symbolId("constant");
    Production production {1, { 2, 3, 4 }, 0 };
    LR1Item item { production, std::vector<int>{ la }, grammar };

    EXPECT_THAT(item.advance().getVisited(), SizeIs(1));
    EXPECT_THAT(item.advance().advance().getExpectedSymbols(), SizeIs(1));
    EXPECT_THAT(item.advance().advance().advance().getExpectedSymbols(), SizeIs(0));
}

TEST(LR1Item, throwsAnExceptionIfAdvancedPastProductionBounds) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/expression_grammar.bnf"));
    const int la = grammar.symbolId("(");
    Production production {1, { 2 }, 0 };
    LR1Item item { production, std::vector<int>{ la }, grammar };

    EXPECT_THAT(item.advance().getVisited(), SizeIs(1));
    ASSERT_THROW(item.advance().advance(), std::out_of_range);
}

TEST(LR1Item, coreKeyTracksProductionAndDot) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/expression_grammar.bnf"));
    const int la1 = grammar.symbolId("identifier");
    const int la2 = grammar.symbolId("constant");
    Production production { 1, { 2, 3 }, 0 };
    LR1Item a { production, std::vector<int>{ la1 }, grammar };
    LR1Item b { production, std::vector<int>{ la2 }, grammar };

    // Same core (production + dot), independent of lookaheads.
    EXPECT_THAT(a.coreKey(), Eq(b.coreKey()));
    EXPECT_THAT(a.advance().coreKey(), Ne(a.coreKey()));
    EXPECT_THAT(a.lookaheads(), Ne(b.lookaheads()));
}
