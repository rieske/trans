#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <stddef.h>
#include <iostream>
#include <memory>
#include <vector>

#include "GrammarSymbol.h"

class LR1Item;

class Grammar {
public:
	virtual ~Grammar();

	LR1Item getReductionById(size_t nonterminalId, size_t productionId) const;

	virtual std::vector<const GrammarSymbol*> getTerminals() const = 0;
	virtual std::vector<const GrammarSymbol*> getNonterminals() const = 0;
	const GrammarSymbol* getStartSymbol() const;
	const GrammarSymbol* getEndSymbol() const;

protected:
	const std::unique_ptr<GrammarSymbol> startSymbol { new GrammarSymbol("<__start__>", 0) };

private:
	const std::unique_ptr<GrammarSymbol> endSymbol { new GrammarSymbol("'$end$'", 0) };
};

std::ostream& operator<<(std::ostream& out, const Grammar& grammar);

#endif // _GRAMMAR_H_
