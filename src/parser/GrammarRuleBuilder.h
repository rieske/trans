#ifndef GRAMMARRULEBUILDER_H_
#define GRAMMARRULEBUILDER_H_

#include <memory>
#include <vector>

class GrammarRule;
class GrammarSymbol;

class GrammarRuleBuilder {
public:
	GrammarRuleBuilder();
	virtual ~GrammarRuleBuilder();

	std::shared_ptr<GrammarRule> build();

	void setDefiningNonterminal(const std::shared_ptr<GrammarSymbol> definingNonterminal);
	void addProductionSymbol(const std::shared_ptr<GrammarSymbol> productionSymbol);

private:
	int ruleId { 1 };

	std::shared_ptr<GrammarSymbol> definingNonterminal;
	std::vector<std::shared_ptr<GrammarSymbol>> production;
};

#endif /* GRAMMARRULEBUILDER_H_ */
