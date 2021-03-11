#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/LR1Item.h"
#include "parser/GrammarSymbol.h"
#include "parser/Production.h"

#include <sstream>
#include <stdexcept>

using namespace testing;
using namespace parser;

TEST(LR1Item, constructsItemFromGrammarRuleAndLookahead) {
    GrammarSymbol nonterm { 1 };
    GrammarSymbol terminal1 { 2 };
    GrammarSymbol nonterm1 { 3 };
    GrammarSymbol terminal2 { 4 };
    Production production { nonterm, { terminal1, nonterm1, terminal2 }, 0 };
    GrammarSymbol lookahead { 5 };
    LR1Item item { production, { lookahead } };

    EXPECT_THAT(item.getDefiningSymbol(), Eq(nonterm));
    EXPECT_THAT(item.getVisited(), SizeIs(0));
    EXPECT_THAT(item.getExpectedSymbols(), SizeIs(3));
    EXPECT_THAT(item.getLookaheads(), Contains(lookahead));
}

TEST(LR1Item, advancesTheVisitedSymbols) {
    GrammarSymbol nonterm { 1 };
    GrammarSymbol terminal1 { 2 };
    GrammarSymbol nonterm1 { 3 };
    GrammarSymbol terminal2 { 4 };
    Production production {nonterm, { terminal1, nonterm1, terminal2 }, 0 };
    GrammarSymbol lookahead { 5 };
    LR1Item item { production, { lookahead } };

    LR1Item advanced1Item = item.advance();
    EXPECT_THAT(advanced1Item.getVisited(), SizeIs(1));
    EXPECT_THAT(advanced1Item.getExpectedSymbols(), SizeIs(2));

    LR1Item advanced2Item = advanced1Item.advance();
    EXPECT_THAT(advanced2Item.getVisited(), SizeIs(2));
    EXPECT_THAT(advanced2Item.getExpectedSymbols(), SizeIs(1));

    LR1Item advanced3Item = advanced2Item.advance();
    EXPECT_THAT(advanced3Item.getVisited(), SizeIs(3));
    EXPECT_THAT(advanced3Item.getExpectedSymbols(), SizeIs(0));
}

TEST(LR1Item, throwsAnExceptionIfAdvancedPastProductionBounds) {
    GrammarSymbol nonterm { 1 };
    GrammarSymbol terminal1 { 2 };
    Production production {nonterm, { terminal1 }, 0 };
    GrammarSymbol lookahead { 3 };
    LR1Item item { production, { lookahead } };

    EXPECT_THAT(item.advance().getVisited(), SizeIs(1));
    EXPECT_THAT(item.advance().getExpectedSymbols(), SizeIs(0));

    ASSERT_THROW(item.advance().advance(), std::out_of_range);
}

