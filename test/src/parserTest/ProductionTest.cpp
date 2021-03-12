#include "parser/Production.h"
#include "parser/GrammarSymbol.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

using namespace parser;

using testing::Eq;

TEST(Production, isConstructedUsingAVector) {
    GrammarSymbol grammarSymbol { 1 };
    Production production { grammarSymbol, {grammarSymbol.getId()}, 0 };

    EXPECT_THAT(production.getDefiningSymbol(), Eq(grammarSymbol));
    EXPECT_THAT(production.producedSequence(), Eq(std::vector<int> { 1 }));
    EXPECT_THAT(production.getId(), Eq(0));
}

TEST(Production, canNotBeConstructedFromEmptyVector) {
    EXPECT_THROW(Production( { 1 }, {}, 0), std::invalid_argument);
}

TEST(Production, allowsIterationOverTheSymbols) {
    GrammarSymbol firstSymbol { 1 };
    GrammarSymbol secondSymbol { 2 };
    Production production { firstSymbol, { firstSymbol.getId(), secondSymbol.getId() }, 0 };

    int index = 0;
    for (const auto& symbol : production) {
        if (index == 0) {
            EXPECT_THAT(symbol, Eq(firstSymbol));
        } else {
            EXPECT_THAT(symbol, Eq(secondSymbol));
        }
        ++index;
    }
}

TEST(Production, returnsProductionSize) {
    GrammarSymbol firstSymbol { 1 };
    GrammarSymbol secondSymbol { 2 };
    Production production { firstSymbol, { firstSymbol.getId(), secondSymbol.getId() }, 0 };

    EXPECT_THAT(production.size(), Eq(2));
}

TEST(Production, returnsProducedSequence) {
    GrammarSymbol firstSymbol { 1 };
    GrammarSymbol secondSymbol { 2 };
    Production production { firstSymbol, { firstSymbol.getId(), secondSymbol.getId() }, 0 };

    EXPECT_THAT(production.producedSequence(), Eq(std::vector<int> { 1, 2 }));
}

