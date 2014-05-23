#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <stddef.h>
#include <iostream>
#include <memory>
#include <vector>

class GrammarSymbol;
class LR1Item;

class Grammar {
public:
	Grammar(const std::vector<std::shared_ptr<const GrammarSymbol>> terminals, const std::vector<std::shared_ptr<const GrammarSymbol>> nonterminals);
	~Grammar();

	LR1Item getReductionById(size_t nonterminalId, size_t productionId) const;

	const std::vector<std::shared_ptr<const GrammarSymbol>> terminals;
	const std::vector<std::shared_ptr<const GrammarSymbol>> nonterminals;

	// FIXME: make me const
	const std::shared_ptr<GrammarSymbol> startSymbol;
	const std::shared_ptr<const GrammarSymbol> endSymbol;
};

std::ostream& operator<<(std::ostream& out, const Grammar& grammar);

#endif // _GRAMMAR_H_
