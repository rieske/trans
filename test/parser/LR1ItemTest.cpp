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
	//LR1Item item;
}
