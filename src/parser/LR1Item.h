#ifndef _ITEM_H_
#define _ITEM_H_

#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>

#include "GrammarSymbol.h"

// [ definingSymbol -> visited . expected, lookaheads ]

class LR1Item {
public:
	LR1Item(const GrammarSymbol* definingGrammarSymbol, size_t productionId, const std::vector<const GrammarSymbol*>& lookaheads);
	virtual ~LR1Item();

	void advance();
	void mergeLookaheads(const std::vector<const GrammarSymbol*>& lookaheadsToMerge);

	std::string getDefiningSymbol() const;
	std::vector<const GrammarSymbol*> getVisited() const;
	std::vector<const GrammarSymbol*> getExpectedSymbols() const;
	std::vector<const GrammarSymbol*> getLookaheads() const;

	size_t getProductionNumber() const;
	Production getProduction() const;

	std::string productionStr() const;

	bool coresAreEqual(const LR1Item& that) const;
	bool operator==(const LR1Item& rhs) const;

private:
	std::string definingSymbol;
	size_t productionNumber { 0 };
	const Production production;
	size_t visitedOffset { 0 };
	std::vector<const GrammarSymbol*> lookaheads;
};

std::ostream& operator<<(std::ostream& out, const LR1Item& item);

#endif // _ITEM_H_
