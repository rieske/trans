#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/LR1Item.h"
#include "parser/GrammarSymbol.h"

#include <sstream>
#include <stdexcept>

using namespace testing;

TEST(LR1Item, constructsItemFromGrammarRuleAndLookahead) {
	auto nonterm = std::make_shared<GrammarSymbol>("<nonterm>", 0);
	std::vector<std::shared_ptr<const GrammarSymbol>> production { std::make_shared<GrammarSymbol>("terminal1", 0),
			std::make_shared<GrammarSymbol>("<nonterm1>", 0), std::make_shared<GrammarSymbol>("terminal2", 0) };
	nonterm->addProduction(production);
	std::shared_ptr<GrammarSymbol> lookahead = std::make_shared<GrammarSymbol>("lookahead", 0);
	LR1Item item { nonterm, 0, { lookahead } };

	ASSERT_THAT(item.getDefiningSymbol()->getName(), Eq("<nonterm>"));
	ASSERT_THAT(item.getVisited(), SizeIs(0));
	ASSERT_THAT(item.getExpectedSymbols(), SizeIs(3));
	ASSERT_THAT(item.getLookaheads(), Contains(lookahead));
}

TEST(LR1Item, advancesTheVisitedSymbols) {
	auto nonterm = std::make_shared<GrammarSymbol>("<nonterm>", 0);
	std::vector<std::shared_ptr<const GrammarSymbol>> production { std::make_shared<GrammarSymbol>("terminal1", 0),
			std::make_shared<GrammarSymbol>("<nonterm1>", 0), std::make_shared<GrammarSymbol>("terminal2", 0) };
	nonterm->addProduction(production);
	std::shared_ptr<GrammarSymbol> lookahead = std::make_shared<GrammarSymbol>("lookahead", 0);
	LR1Item item { nonterm, 0, { lookahead } };

	item.advance();
	ASSERT_THAT(item.getVisited(), SizeIs(1));
	ASSERT_THAT(item.getExpectedSymbols(), SizeIs(2));

	item.advance();
	ASSERT_THAT(item.getVisited(), SizeIs(2));
	ASSERT_THAT(item.getExpectedSymbols(), SizeIs(1));

	item.advance();
	ASSERT_THAT(item.getVisited(), SizeIs(3));
	ASSERT_THAT(item.getExpectedSymbols(), SizeIs(0));
}

TEST(LR1Item, throwsAnExceptionIfAdvancedPastProductionBounds) {
	auto nonterm = std::make_shared<GrammarSymbol>("<nonterm>", 0);
	std::vector<std::shared_ptr<const GrammarSymbol>> production { std::make_shared<GrammarSymbol>("terminal1", 0) };
	nonterm->addProduction(production);
	std::shared_ptr<GrammarSymbol> lookahead = std::make_shared<GrammarSymbol>("lookahead", 0);
	LR1Item item { nonterm, 0, { lookahead } };

	item.advance();
	ASSERT_THAT(item.getVisited(), SizeIs(1));
	ASSERT_THAT(item.getExpectedSymbols(), SizeIs(0));

	ASSERT_THROW(item.advance(), std::out_of_range);
}

TEST(LR1Item, outputsTheItem) {
	auto nonterm = std::make_shared<GrammarSymbol>("<nonterm>", 0);
	std::vector<std::shared_ptr<const GrammarSymbol>> production { std::make_shared<GrammarSymbol>("terminal1", 0),
			std::make_shared<GrammarSymbol>("<nonterm1>", 0), std::make_shared<GrammarSymbol>("terminal2", 0) };
	nonterm->addProduction(production);
	std::shared_ptr<GrammarSymbol> lookahead = std::make_shared<GrammarSymbol>("lookahead", 0);
	LR1Item item { nonterm, 0, { lookahead } };

	std::stringstream sstream;
	sstream << item;
	ASSERT_THAT(sstream.str(), Eq("[ <nonterm> -> . terminal1 <nonterm1> terminal2 , lookahead ]\n"));

	sstream.str("");
	item.advance();
	sstream << item;
	ASSERT_THAT(sstream.str(), Eq("[ <nonterm> -> terminal1 . <nonterm1> terminal2 , lookahead ]\n"));

	sstream.str("");
	item.advance();
	sstream << item;
	ASSERT_THAT(sstream.str(), Eq("[ <nonterm> -> terminal1 <nonterm1> . terminal2 , lookahead ]\n"));

	sstream.str("");
	item.advance();
	sstream << item;
	ASSERT_THAT(sstream.str(), Eq("[ <nonterm> -> terminal1 <nonterm1> terminal2 . , lookahead ]\n"));
}
