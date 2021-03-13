#include "parser/Production.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

using namespace parser;
using namespace testing;

TEST(Production, isConstructedUsingAVector) {
    Production production { 1, {1}, 0 };

    EXPECT_THAT(production.getDefiningSymbol(), Eq(1));
    EXPECT_THAT(production.producedSequence(), ElementsAre(1));
    EXPECT_THAT(production.getId(), Eq(0));
}

TEST(Production, canNotBeConstructedFromEmptyVector) {
    EXPECT_THROW(Production( 1, {}, 0), std::invalid_argument);
}

TEST(Production, allowsIterationOverTheSymbols) {
    Production production { 1, { 1, 2 }, 0 };

    EXPECT_THAT(production.producedSequence(), ElementsAre(1, 2));
    int index = 0;
    for (const auto& symbol : production) {
        if (index == 0) {
            EXPECT_THAT(symbol, Eq(1));
        } else {
            EXPECT_THAT(symbol, Eq(2));
        }
        ++index;
    }
}

TEST(Production, returnsProductionSize) {
    Production production { 1, { 1, 2 }, 0 };

    EXPECT_THAT(production.size(), Eq(2));
}

TEST(Production, returnsProducedSequence) {
    Production production { 1, { 1, 2 }, 0 };

    EXPECT_THAT(production.producedSequence(), ElementsAre(1, 2));
}

