#include "parser/Production.h"
#include "parser/GrammarSymbol.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

using namespace parser;

using testing::Eq;

TEST(Production, isConstructedUsingAVector) {
	std::unique_ptr<GrammarSymbol> grammarSymbol { new GrammarSymbol { "<symbol>" } };
	std::vector<const GrammarSymbol*> symbolSequence { grammarSymbol.get() };
	Production production { symbolSequence };
}

TEST(Production, canNotBeConstructedFromEmptyVector) {
	std::vector<const GrammarSymbol*> emptySymbolSequence { };
	EXPECT_THROW(Production { emptySymbolSequence }, std::invalid_argument);
}

TEST(Production, allowsIterationOverTheSymbols) {
	std::unique_ptr<GrammarSymbol> firstSymbol { new GrammarSymbol { "<symbol1>" } };
	std::unique_ptr<GrammarSymbol> secondSymbol { new GrammarSymbol { "<symbol2>" } };
	std::vector<const GrammarSymbol*> symbolSequence { firstSymbol.get(), secondSymbol.get() };
	Production production { symbolSequence };

	int index = 0;
	for (const GrammarSymbol* symbol : production) {
		if (index == 0) {
			EXPECT_THAT(symbol, Eq(firstSymbol.get()));
		} else {
			EXPECT_THAT(symbol, Eq(secondSymbol.get()));
		}
		++index;
	}
}

TEST(Production, returnsProductionSize) {
	std::unique_ptr<GrammarSymbol> firstSymbol { new GrammarSymbol { "<symbol1>" } };
	std::unique_ptr<GrammarSymbol> secondSymbol { new GrammarSymbol { "<symbol2>" } };
	std::vector<const GrammarSymbol*> symbolSequence { firstSymbol.get(), secondSymbol.get() };
	Production production { symbolSequence };

	EXPECT_THAT(production.size(), Eq(2));
}

TEST(Production, canBeComparedToStringSequence) {
	std::unique_ptr<GrammarSymbol> firstSymbol { new GrammarSymbol { "<symbol1>" } };
	std::unique_ptr<GrammarSymbol> secondSymbol { new GrammarSymbol { "<symbol2>" } };
	std::vector<const GrammarSymbol*> symbolSequence { firstSymbol.get(), secondSymbol.get() };
	Production production { symbolSequence };

	EXPECT_THAT(production.produces( { "<symbol1>", "<symbol2>" }), Eq(true));
	EXPECT_THAT(production.produces( { "<symbol1>", "<symbol2>", "<symbol3>" }), Eq(false));
	EXPECT_THAT(production.produces( { "<symbol3>", "<symbol1>", "<symbol2>" }), Eq(false));
	EXPECT_THAT(production.produces( { "<symbol1>", "<symbol3>", "<symbol2>" }), Eq(false));
	EXPECT_THAT(production.produces( { "<symbol1>" }), Eq(false));
	EXPECT_THAT(production.produces( { "<symbol2>" }), Eq(false));
	EXPECT_THAT(production.produces( { "<symbol2>", "<symbol1>" }), Eq(false));
	EXPECT_THAT(production.produces( { "<aaa>", "<bbb>" }), Eq(false));
}
