#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/LR1Item.h"
#include "parser/GrammarSymbol.h"

#include <sstream>
#include <stdexcept>

using namespace testing;
using std::unique_ptr;

TEST(LR1Item, constructsItemFromGrammarRuleAndLookahead) {
	unique_ptr<GrammarSymbol> nonterm { new GrammarSymbol("<nonterm>") };
	unique_ptr<GrammarSymbol> terminal1 { new GrammarSymbol("terminal1") };
	unique_ptr<GrammarSymbol> nonterm1 { new GrammarSymbol("<nonterm1>") };
	unique_ptr<GrammarSymbol> terminal2 { new GrammarSymbol("terminal2") };
	Production production { terminal1.get(), nonterm1.get(), terminal2.get() };
	nonterm->addProduction(production);
	unique_ptr<GrammarSymbol> lookahead { new GrammarSymbol("lookahead") };
	LR1Item item { nonterm.get(), 0, { lookahead.get() } };

	ASSERT_THAT(item.getDefiningSymbol()->getSymbol(), Eq("<nonterm>"));
	ASSERT_THAT(item.getVisited(), SizeIs(0));
	ASSERT_THAT(item.getExpectedSymbols(), SizeIs(3));
	ASSERT_THAT(item.getLookaheads(), Contains(lookahead.get()));
}

TEST(LR1Item, advancesTheVisitedSymbols) {
	unique_ptr<GrammarSymbol> nonterm { new GrammarSymbol("<nonterm>") };
	unique_ptr<GrammarSymbol> terminal1 { new GrammarSymbol("terminal1") };
	unique_ptr<GrammarSymbol> nonterm1 { new GrammarSymbol("<nonterm1>") };
	unique_ptr<GrammarSymbol> terminal2 { new GrammarSymbol("terminal2") };
	Production production { terminal1.get(), nonterm1.get(), terminal2.get() };
	nonterm->addProduction(production);
	unique_ptr<GrammarSymbol> lookahead { new GrammarSymbol("lookahead") };
	LR1Item item { nonterm.get(), 0, { lookahead.get() } };

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
	unique_ptr<GrammarSymbol> nonterm { new GrammarSymbol("<nonterm>") };
	unique_ptr<GrammarSymbol> terminal1 { new GrammarSymbol("terminal1") };
	Production production { terminal1.get() };
	nonterm->addProduction(production);
	std::shared_ptr<GrammarSymbol> lookahead = std::make_shared<GrammarSymbol>("lookahead");
	LR1Item item { nonterm.get(), 0, { lookahead.get() } };

	item.advance();
	ASSERT_THAT(item.getVisited(), SizeIs(1));
	ASSERT_THAT(item.getExpectedSymbols(), SizeIs(0));

	ASSERT_THROW(item.advance(), std::out_of_range);
}

TEST(LR1Item, outputsTheItem) {
	unique_ptr<GrammarSymbol> nonterm { new GrammarSymbol("<nonterm>") };
	unique_ptr<GrammarSymbol> terminal1 { new GrammarSymbol("terminal1") };
	unique_ptr<GrammarSymbol> nonterm1 { new GrammarSymbol("<nonterm1>") };
	unique_ptr<GrammarSymbol> terminal2 { new GrammarSymbol("terminal2") };
	Production production { terminal1.get(), nonterm1.get(), terminal2.get() };
	nonterm->addProduction(production);
	unique_ptr<GrammarSymbol> lookahead { new GrammarSymbol("lookahead") };
	LR1Item item { nonterm.get(), 0, { lookahead.get() } };

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
