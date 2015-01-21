#ifndef _ITEM_H_
#define _ITEM_H_

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include "BNFFileGrammar.h"
#include "Production.h"

namespace parser {

// [ definingSymbol -> visited . expected, lookaheads ]

class LR1Item {
public:
	LR1Item(const GrammarSymbol* definingSymbol, size_t productionId, const std::vector<const GrammarSymbol*>& lookaheads);
	virtual ~LR1Item();

	LR1Item advance() const;
	void mergeLookaheads(const std::vector<const GrammarSymbol*>& lookaheadsToMerge);

	const GrammarSymbol* getDefiningSymbol() const;
	std::vector<const GrammarSymbol*> getVisited() const;
	bool hasUnvisitedSymbols() const;
	const GrammarSymbol* nextUnvisitedSymbol() const;
    std::vector<const GrammarSymbol*> getExpectedSymbols() const;
	std::vector<const GrammarSymbol*> getLookaheads() const;

	size_t getProductionNumber() const;
	Production getProduction() const;

	bool coresAreEqual(const LR1Item& that) const;
	bool operator==(const LR1Item& rhs) const;

private:

	const GrammarSymbol* definingSymbol;
	size_t productionNumber;
	const Production production;
	size_t visitedOffset { 0 };
	std::vector<const GrammarSymbol*> lookaheads;
};

std::ostream& operator<<(std::ostream& out, const LR1Item& item);

}

#endif // _ITEM_H_
