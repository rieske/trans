#ifndef _ITEM_H_
#define _ITEM_H_

#include <iostream>
#include <memory>
#include <vector>

class GrammarSymbol;

/**
 * @author Vaidotas Valuckas
 * LR(1) Item
 * [ left -> seen . expected, lookaheads ]
 **/

class Item {
public:
	Item(std::shared_ptr<GrammarSymbol> r);
	~Item();

	void addSeen(std::shared_ptr<GrammarSymbol> symbol);
	void addExpected(std::shared_ptr<GrammarSymbol> expectedSymbol);
	void setExpected(std::vector<std::shared_ptr<GrammarSymbol>> expected);
	void addLookahead(std::shared_ptr<GrammarSymbol>);
	void mergeLookaheads(const Item& item);

	std::shared_ptr<GrammarSymbol> getLeft() const;
	std::vector<std::shared_ptr<GrammarSymbol>> getSeen() const;
	std::vector<std::shared_ptr<GrammarSymbol>> getExpected() const;
	std::vector<std::shared_ptr<GrammarSymbol>> getLookaheads() const;

	void print() const;

	void log(std::ostream &out) const;

	bool coresAreEqual(Item& rhs) const;

	bool operator==(const Item& rhs) const;
	bool operator!=(const Item& rhs) const;

private:
	std::shared_ptr<GrammarSymbol> left;
	std::vector<std::shared_ptr<GrammarSymbol>> seen;
	std::vector<std::shared_ptr<GrammarSymbol>> expected;
	std::vector<std::shared_ptr<GrammarSymbol>> lookaheads;
};

#endif // _ITEM_H_
