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
	LR1Item(std::shared_ptr<const GrammarSymbol> definingSymbol, size_t productionId, std::vector<std::shared_ptr<const GrammarSymbol>> lookaheads);
	virtual ~LR1Item();

	void advance();
	void mergeLookaheads(const std::vector<std::shared_ptr<const GrammarSymbol>>& lookaheadsToMerge);

	std::shared_ptr<const GrammarSymbol> getDefiningSymbol() const;
	std::vector<std::shared_ptr<const GrammarSymbol>> getVisited() const;
	std::vector<std::shared_ptr<const GrammarSymbol>> getExpected() const;
	std::vector<std::shared_ptr<const GrammarSymbol>> getLookaheads() const;

	size_t getProductionId() const;
	Production getProduction() const;

	std::string productionStr() const;

	bool coresAreEqual(const LR1Item& that) const;
	bool operator==(const LR1Item& rhs) const;

private:
	std::shared_ptr<const GrammarSymbol> definingSymbol;
	size_t productionId { 0 };
	size_t visitedOffset { 0 };
	std::vector<std::shared_ptr<const GrammarSymbol>> lookaheads;
};

std::ostream& operator<<(std::ostream& out, const LR1Item& item);

#endif // _ITEM_H_
