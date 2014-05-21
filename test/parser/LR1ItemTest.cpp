#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/LR1Item.h"
#include "parser/GrammarRuleBuilder.h"
#include "parser/NonterminalSymbol.h"
#include "parser/TerminalSymbol.h"
#include "parser/GrammarRule.h"

#include <sstream>

using namespace testing;

TEST(LR1Item, constructsItemFromGrammarRuleAndLookahead) {
	GrammarRuleBuilder ruleBuilder;
	ruleBuilder.setDefiningNonterminal(std::make_shared<NonterminalSymbol>("<nonterm>"));
	ruleBuilder.addProductionSymbol(std::make_shared<TerminalSymbol>("terminal1"));
	ruleBuilder.addProductionSymbol(std::make_shared<NonterminalSymbol>("<nonterm1>"));
	ruleBuilder.addProductionSymbol(std::make_shared<TerminalSymbol>("terminal2"));
	std::shared_ptr<GrammarSymbol> lookahead = std::make_shared<TerminalSymbol>("lookahead");
	auto rule = ruleBuilder.build();
	LR1Item item { rule->getNonterminal(), *rule,  { lookahead } };

	ASSERT_THAT(item.getDefiningSymbol()->getName(), Eq("<nonterm>"));
	ASSERT_THAT(item.getVisited(), SizeIs(0));
	ASSERT_THAT(item.getExpected(), SizeIs(3));
	ASSERT_THAT(item.getLookaheads(), Contains(lookahead));
}

TEST(LR1Item, advancesTheVisitedSymbols) {
	GrammarRuleBuilder ruleBuilder;
	ruleBuilder.setDefiningNonterminal(std::make_shared<NonterminalSymbol>("<nonterm>"));
	ruleBuilder.addProductionSymbol(std::make_shared<TerminalSymbol>("terminal1"));
	ruleBuilder.addProductionSymbol(std::make_shared<NonterminalSymbol>("<nonterm1>"));
	ruleBuilder.addProductionSymbol(std::make_shared<TerminalSymbol>("terminal2"));
	std::shared_ptr<GrammarSymbol> lookahead = std::make_shared<TerminalSymbol>("lookahead");
	auto rule = ruleBuilder.build();
	LR1Item item { rule->getNonterminal(), *rule, { lookahead } };

	item.advance();
	ASSERT_THAT(item.getVisited(), SizeIs(1));
	ASSERT_THAT(item.getExpected(), SizeIs(2));

	item.advance();
	ASSERT_THAT(item.getVisited(), SizeIs(2));
	ASSERT_THAT(item.getExpected(), SizeIs(1));

	item.advance();
	ASSERT_THAT(item.getVisited(), SizeIs(3));
	ASSERT_THAT(item.getExpected(), SizeIs(0));
}

TEST(LR1Item, throwsAnExceptionIfAdvancedPastProductionBounds) {
	GrammarRuleBuilder ruleBuilder;
	ruleBuilder.setDefiningNonterminal(std::make_shared<NonterminalSymbol>("<nonterm>"));
	ruleBuilder.addProductionSymbol(std::make_shared<TerminalSymbol>("terminal1"));

	std::shared_ptr<GrammarSymbol> lookahead = std::make_shared<TerminalSymbol>("lookahead");
	auto rule = ruleBuilder.build();
	LR1Item item { rule->getNonterminal(), *rule, { lookahead } };

	item.advance();
	ASSERT_THAT(item.getVisited(), SizeIs(1));
	ASSERT_THAT(item.getExpected(), SizeIs(0));

	ASSERT_THROW(item.advance(), std::out_of_range);
}

TEST(LR1Item, outputsTheItem) {
	GrammarRuleBuilder ruleBuilder;
	ruleBuilder.setDefiningNonterminal(std::make_shared<NonterminalSymbol>("<nonterm>"));
	ruleBuilder.addProductionSymbol(std::make_shared<TerminalSymbol>("terminal1"));
	ruleBuilder.addProductionSymbol(std::make_shared<NonterminalSymbol>("<nonterm1>"));
	ruleBuilder.addProductionSymbol(std::make_shared<TerminalSymbol>("terminal2"));
	std::shared_ptr<GrammarSymbol> lookahead = std::make_shared<TerminalSymbol>("lookahead");

	auto rule = ruleBuilder.build();
	LR1Item item { rule->getNonterminal(), *rule, { lookahead } };

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
