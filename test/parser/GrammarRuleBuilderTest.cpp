#include "parser/GrammarRuleBuilder.h"

#include "parser/GrammarRule.h"
#include "parser/TerminalSymbol.h"
#include "parser/NonterminalSymbol.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace testing;
using std::shared_ptr;

TEST(GrammarRuleBuilder, buildsGrammarRule) {
	GrammarRuleBuilder builder;

	shared_ptr<GrammarSymbol> definingNonterminal { new NonterminalSymbol { "definingNonterminal" } };
	shared_ptr<GrammarSymbol> productionTerminal { new TerminalSymbol { "productionTerminal" } };
	shared_ptr<GrammarSymbol> productionNonterminal { new NonterminalSymbol { "productionNonterminal" } };
	builder.setDefiningNonterminal(definingNonterminal);
	builder.addProductionSymbol(productionTerminal);
	builder.addProductionSymbol(productionNonterminal);

	shared_ptr<GrammarRule> grammarRule = builder.build();

	ASSERT_THAT(grammarRule->getNonterminal(), Eq(definingNonterminal));
	ASSERT_THAT(grammarRule->getProduction(), ElementsAre(productionTerminal, productionNonterminal));
}

TEST(GrammarRuleBuilder, buildsNextGrammarRuleWithSameNonterminal) {
	GrammarRuleBuilder builder;

	shared_ptr<GrammarSymbol> definingNonterminal { new NonterminalSymbol { "definingNonterminal" } };
	shared_ptr<GrammarSymbol> productionTerminal { new TerminalSymbol { "productionTerminal" } };
	shared_ptr<GrammarSymbol> productionNonterminal { new NonterminalSymbol { "productionNonterminal" } };
	builder.setDefiningNonterminal(definingNonterminal);
	builder.addProductionSymbol(productionTerminal);
	builder.addProductionSymbol(productionNonterminal);
	builder.build();
	builder.addProductionSymbol(productionNonterminal);
	builder.addProductionSymbol(definingNonterminal);

	shared_ptr<GrammarRule> grammarRule = builder.build();

	ASSERT_THAT(grammarRule->getNonterminal(), Eq(definingNonterminal));
	ASSERT_THAT(grammarRule->getProduction(), ElementsAre(productionNonterminal, definingNonterminal));
}
