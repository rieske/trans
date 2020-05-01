#include "../parser/Production.h"
#include "../parser/GrammarSymbol.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

using namespace parser;

using testing::Eq;

TEST(Production, isConstructedUsingAVector) {
    GrammarSymbol grammarSymbol { "<symbol>" };
    std::vector<GrammarSymbol> symbolSequence { grammarSymbol };
    Production production { grammarSymbol, symbolSequence, 0 };

    EXPECT_THAT(production.getDefiningSymbol(), Eq(grammarSymbol));
    EXPECT_THAT(production.producedSequence(), Eq(std::vector<std::string> { "<symbol>" }));
    EXPECT_THAT(production.getId(), Eq(0));
}

TEST(Production, canNotBeConstructedFromEmptyVector) {
    std::vector<GrammarSymbol> emptySymbolSequence { };
    EXPECT_THROW(Production( { "" }, emptySymbolSequence, 0), std::invalid_argument);
}

TEST(Production, allowsIterationOverTheSymbols) {
    GrammarSymbol firstSymbol { "<symbol1>" };
    GrammarSymbol secondSymbol { "<symbol2>" };
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
    GrammarSymbol firstSymbol { "<symbol1>" };
    GrammarSymbol secondSymbol { "<symbol2>" };
    std::vector<GrammarSymbol> symbolSequence { firstSymbol, secondSymbol };
    Production production { firstSymbol, symbolSequence, 0 };

    EXPECT_THAT(production.size(), Eq(2));
}

TEST(Production, canBeComparedToStringSequence) {
    GrammarSymbol firstSymbol { "<symbol1>" };
    GrammarSymbol secondSymbol { "<symbol2>" };
    std::vector<GrammarSymbol> symbolSequence { firstSymbol, secondSymbol };
    Production production { firstSymbol, symbolSequence, 0 };

    EXPECT_THAT(production.produces( { "<symbol1>", "<symbol2>" }), Eq(true));
    EXPECT_THAT(production.produces( { "<symbol1>", "<symbol2>", "<symbol3>" }), Eq(false));
    EXPECT_THAT(production.produces( { "<symbol3>", "<symbol1>", "<symbol2>" }), Eq(false));
    EXPECT_THAT(production.produces( { "<symbol1>", "<symbol3>", "<symbol2>" }), Eq(false));
    EXPECT_THAT(production.produces( { "<symbol1>" }), Eq(false));
    EXPECT_THAT(production.produces( { "<symbol2>" }), Eq(false));
    EXPECT_THAT(production.produces( { "<symbol2>", "<symbol1>" }), Eq(false));
    EXPECT_THAT(production.produces( { "<aaa>", "<bbb>" }), Eq(false));
}

TEST(Production, returnsProducedSequence) {
    GrammarSymbol firstSymbol { "<symbol1>" };
    GrammarSymbol secondSymbol { "<symbol2>" };
    std::vector<GrammarSymbol> symbolSequence { firstSymbol, secondSymbol };
    Production production { firstSymbol, symbolSequence, 0 };

    EXPECT_THAT(production.producedSequence(), Eq(std::vector<std::string> { "<symbol1>", "<symbol2>" }));
}
