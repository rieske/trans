#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <stddef.h>
#include <iostream>
#include <memory>
#include <vector>

#include "Closure.h"
#include "FirstTable.h"
#include "LR1Item.h"

class Grammar {
public:
	Grammar(const std::vector<std::shared_ptr<GrammarSymbol>> terminals, const std::vector<std::shared_ptr<GrammarSymbol>> nonterminals);
	~Grammar();

	LR1Item getReductionById(size_t nonterminalId, size_t productionId) const;

	std::vector<std::shared_ptr<GrammarSymbol>> getNonterminals() const;
	std::vector<std::shared_ptr<GrammarSymbol>> getTerminals() const;

	std::shared_ptr<GrammarSymbol> getStartSymbol() const;
	std::shared_ptr<GrammarSymbol> getEndSymbol() const;

	std::vector<LR1Item> goTo(const std::vector<LR1Item>& I, const std::shared_ptr<GrammarSymbol> X) const;
	std::vector<std::vector<LR1Item>> canonicalCollection() const;

private:
	std::vector<std::shared_ptr<GrammarSymbol>> terminals;
	std::vector<std::shared_ptr<GrammarSymbol>> nonterminals;

	std::shared_ptr<GrammarSymbol> start_symbol;
	std::shared_ptr<GrammarSymbol> end_symbol;

	Closure closure;

	friend std::ostream& operator<<(std::ostream& out, const Grammar& grammar);
};

#endif // _GRAMMAR_H_
