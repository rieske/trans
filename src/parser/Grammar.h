#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <iostream>
#include <memory>
#include <vector>

#include "LR1Item.h"

class FirstTable;
class GrammarRule;

class Grammar {
public:
	Grammar(const std::vector<std::shared_ptr<GrammarSymbol>> terminals,
			const std::vector<std::shared_ptr<GrammarSymbol>> nonterminals,
			const std::vector<std::shared_ptr<GrammarRule>> rules);
	~Grammar();

	std::shared_ptr<GrammarRule> getRuleById(int ruleId) const;
	std::shared_ptr<GrammarRule> getRuleByDefinition(const std::shared_ptr<GrammarSymbol> nonterminal,
			const std::vector<std::shared_ptr<GrammarSymbol>>& production) const;

	std::vector<std::shared_ptr<GrammarSymbol>> getNonterminals() const;
	std::vector<std::shared_ptr<GrammarSymbol>> getTerminals() const;

	std::shared_ptr<GrammarSymbol> getStartSymbol() const;
	std::shared_ptr<GrammarSymbol> getEndSymbol() const;

	std::vector<LR1Item> go_to(std::vector<LR1Item> I, std::shared_ptr<GrammarSymbol> X) const;
	std::vector<std::vector<LR1Item>> canonical_collection() const;

private:
	std::vector<LR1Item> closure(std::vector<LR1Item> I) const;

	std::vector<std::shared_ptr<GrammarSymbol>> terminals;
	std::vector<std::shared_ptr<GrammarSymbol>> nonterminals;
	std::vector<std::shared_ptr<GrammarRule>> rules;

	std::shared_ptr<GrammarSymbol> start_symbol;
	std::shared_ptr<GrammarSymbol> end_symbol;

	std::unique_ptr<FirstTable> firstTable;

	friend std::ostream& operator<<(std::ostream& out, const Grammar& grammar);
};

#endif // _GRAMMAR_H_
