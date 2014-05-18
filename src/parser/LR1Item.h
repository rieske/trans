#ifndef _ITEM_H_
#define _ITEM_H_

#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>

class GrammarRule;
class GrammarSymbol;

// [ definingSymbol -> visited . expected, lookaheads ]

class LR1Item {
public:
	LR1Item(std::shared_ptr<GrammarRule> rule, std::shared_ptr<GrammarSymbol> lookahead);
	virtual ~LR1Item();

	void advance();
	void mergeLookaheads(const LR1Item& item);

	std::shared_ptr<GrammarSymbol> getDefiningSymbol() const;
	std::vector<std::shared_ptr<GrammarSymbol>> getVisited() const;
	std::vector<std::shared_ptr<GrammarSymbol>> getExpected() const;
	std::vector<std::shared_ptr<GrammarSymbol>> getLookaheads() const;

	bool coresAreEqual(const LR1Item& that) const;
	bool operator==(const LR1Item& rhs) const;

private:
	std::shared_ptr<GrammarRule> rule;
	size_t visitedOffset { 0 };
	std::vector<std::shared_ptr<GrammarSymbol>> lookaheads;
};

std::ostream& operator<<(std::ostream& out, const LR1Item& item);

#endif // _ITEM_H_
