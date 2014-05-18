#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/LR1Item.h"
#include "parser/GrammarRuleBuilder.h"
#include "parser/NonterminalSymbol.h"
#include "parser/TerminalSymbol.h"

using namespace testing;

TEST(LR1Item, constructsItemFromGrammarRuleAndLookahead) {
	GrammarRuleBuilder ruleBuilder;
	ruleBuilder.setDefiningNonterminal(std::make_shared<NonterminalSymbol>("<nonterm>"));
	ruleBuilder.addProductionSymbol(std::make_shared<TerminalSymbol>("terminal1"));
	ruleBuilder.addProductionSymbol(std::make_shared<NonterminalSymbol>("<nonterm1>"));
	ruleBuilder.addProductionSymbol(std::make_shared<TerminalSymbol>("terminal2"));
	std::shared_ptr<GrammarSymbol> lookahead = std::make_shared<TerminalSymbol>("lookahead");
	LR1Item item { ruleBuilder.build(), lookahead };

	ASSERT_THAT(item.getDefiningSymbol()->getName(), Eq("<nonterm>"));
	ASSERT_THAT(item.getVisited(), SizeIs(0));
	ASSERT_THAT(item.getExpected(), SizeIs(3));
	ASSERT_THAT(item.getLookaheads(), Contains(lookahead));
}
