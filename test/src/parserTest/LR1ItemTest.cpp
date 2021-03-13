#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/LR1Item.h"
#include "parser/Production.h"

#include <stdexcept>

using namespace testing;
using namespace parser;

TEST(LR1Item, constructsItemFromGrammarRuleAndLookahead) {
    Production production { 1, { 2, 3, 4 }, 0 };
    LR1Item item { production, { 5 } };

    EXPECT_THAT(item.getProduction().getDefiningSymbol(), Eq(1));
    EXPECT_THAT(item.getVisited(), SizeIs(0));
    EXPECT_THAT(item.getExpectedSymbols(), SizeIs(3));
    EXPECT_THAT(item.getLookaheads(), ElementsAre(5));
}

TEST(LR1Item, advancesTheVisitedSymbols) {
    Production production {1, { 2, 3, 4 }, 0 };
    LR1Item item { production, { 5 } };

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
    Production production {1, { 2 }, 0 };
    LR1Item item { production, { 3 } };

    EXPECT_THAT(item.advance().getVisited(), SizeIs(1));
    EXPECT_THAT(item.advance().getExpectedSymbols(), SizeIs(0));

    ASSERT_THROW(item.advance().advance(), std::out_of_range);
}

