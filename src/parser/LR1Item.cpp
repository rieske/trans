#include "LR1Item.h"

#include <iterator>

#include "GrammarRule.h"
#include "GrammarSymbol.h"

using std::vector;
using std::shared_ptr;

LR1Item::LR1Item(shared_ptr<GrammarRule> rule, shared_ptr<GrammarSymbol> lookahead) :
		rule { rule } {
	lookaheads.push_back(lookahead);
}

LR1Item::~LR1Item() {
}

void LR1Item::advance() {
	++visitedOffset;
}

bool LR1Item::operator==(const LR1Item& rhs) const {
	return (this->rule == rhs.rule) && (this->visitedOffset == rhs.visitedOffset) && (this->lookaheads == rhs.lookaheads);
}

bool LR1Item::coresAreEqual(const LR1Item& that) const {
	return (this->rule == that.rule) && (this->visitedOffset == that.visitedOffset);
}

void LR1Item::mergeLookaheads(const LR1Item& item) {
	if (lookaheads.empty()) {
		for (unsigned i = 0; i < item.lookaheads.size(); i++)
			lookaheads.push_back(item.lookaheads.at(i));
		return;
	}
	for (unsigned i = 0; i < item.lookaheads.size(); i++) {
		bool insert = true;
		unsigned j = 0;
		for (; j < this->lookaheads.size(); j++) {
			if (item.lookaheads.at(i) == this->lookaheads.at(j)) {
				insert = false;
				break;
			}
		}
		if (insert)
			this->lookaheads.push_back(item.lookaheads.at(i));
	}
}

shared_ptr<GrammarSymbol> LR1Item::getDefiningSymbol() const {
	return rule->getNonterminal();
}

vector<shared_ptr<GrammarSymbol>> LR1Item::getVisited() const {
	const auto& ruleProduction = rule->getProduction();
	return vector<shared_ptr<GrammarSymbol>> { ruleProduction.begin(), ruleProduction.begin() + visitedOffset };
}

vector<shared_ptr<GrammarSymbol>> LR1Item::getExpected() const {
	const auto& ruleProduction = rule->getProduction();
	return vector<shared_ptr<GrammarSymbol>> { ruleProduction.begin() + visitedOffset, ruleProduction.end() };
}

vector<shared_ptr<GrammarSymbol>> LR1Item::getLookaheads() const {
	return lookaheads;
}

std::ostream& operator<<(std::ostream& out, const LR1Item& item) {
	out << "[ " << item.getDefiningSymbol() << " -> ";
	for (const auto& visitedSymbol : item.getVisited()) {
		out << visitedSymbol << " ";
	}
	out << ". ";
	for (const auto& expectedSymbol : item.getExpected()) {
		out << *expectedSymbol << " ";
	}
	out << ", ";
	for (const auto& lookahead : item.getLookaheads()) {
		out << lookahead << " ";
	}
	out << " ]\n";
	return out;
}
