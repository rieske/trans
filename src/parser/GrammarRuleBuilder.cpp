#include "GrammarRuleBuilder.h"

#include "GrammarRule.h"
#include "GrammarSymbol.h"

using std::shared_ptr;

GrammarRuleBuilder::GrammarRuleBuilder() {
}

GrammarRuleBuilder::~GrammarRuleBuilder() {
}

shared_ptr<GrammarRule> GrammarRuleBuilder::build() {
	shared_ptr<GrammarRule> rule { new GrammarRule { definingNonterminal, production, ruleId } };
	production.clear();
	ruleId++;
	return rule;
}

void GrammarRuleBuilder::setDefiningNonterminal(const shared_ptr<GrammarSymbol> definingNonterminal) {
	this->definingNonterminal = definingNonterminal;
}

void GrammarRuleBuilder::addProductionSymbol(const shared_ptr<GrammarSymbol> productionSymbol) {
	production.push_back(productionSymbol);
}
