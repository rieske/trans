#include "parser/Production.h"
#include "parser/GrammarSymbol.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

using namespace parser;

using testing::Eq;

TEST(Production, isConstructedUsingAVector) {
    GrammarSymbol grammarSymbol { 1 };
    std::vector<GrammarSymbol> symbolSequence { grammarSymbol };
    Production production { grammarSymbol, symbolSequence, 0 };

    EXPECT_THAT(production.getDefiningSymbol(), Eq(grammarSymbol));
    EXPECT_THAT(production.producedSequence(), Eq(std::vector<int> { 1 }));
    EXPECT_THAT(production.getId(), Eq(0));
}

TEST(Production, canNotBeConstructedFromEmptyVector) {
    std::vector<GrammarSymbol> emptySymbolSequence { };
    EXPECT_THROW(Production( { 1 }, emptySymbolSequence, 0), std::invalid_argument);
}

TEST(Production, allowsIterationOverTheSymbols) {
    GrammarSymbol firstSymbol { 1 };
    GrammarSymbol secondSymbol { 2 };
    std::vector<GrammarSymbol> symbolSequence { firstSymbol, secondSymbol };
    Production production { firstSymbol, symbolSequence, 0 };

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
    std::vector<GrammarSymbol> symbolSequence { firstSymbol, secondSymbol };
    Production production { firstSymbol, symbolSequence, 0 };

    EXPECT_THAT(production.size(), Eq(2));
}

TEST(Production, canBeComparedToStringSequence) {
    GrammarSymbol firstSymbol { 1 };
    GrammarSymbol secondSymbol { 2 };
    std::vector<GrammarSymbol> symbolSequence { firstSymbol, secondSymbol };
    Production production { firstSymbol, symbolSequence, 0 };

    EXPECT_THAT(production.produces( { "1", "2" }), Eq(true));
    EXPECT_THAT(production.produces( { "1", "2", "3" }), Eq(false));
    EXPECT_THAT(production.produces( { "3", "1", "2" }), Eq(false));
    EXPECT_THAT(production.produces( { "1", "3", "2" }), Eq(false));
    EXPECT_THAT(production.produces( { "1" }), Eq(false));
    EXPECT_THAT(production.produces( { "2" }), Eq(false));
    EXPECT_THAT(production.produces( { "2", "1" }), Eq(false));
    EXPECT_THAT(production.produces( { "<aaa>", "<bbb>" }), Eq(false));
}

TEST(Production, returnsProducedSequence) {
    GrammarSymbol firstSymbol { 1 };
    GrammarSymbol secondSymbol { 2 };
    std::vector<GrammarSymbol> symbolSequence { firstSymbol, secondSymbol };
    Production production { firstSymbol, symbolSequence, 0 };

    EXPECT_THAT(production.producedSequence(), Eq(std::vector<int> { 1, 2 }));
}
