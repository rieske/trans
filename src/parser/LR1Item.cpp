#include "LR1Item.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "GrammarRule.h"
#include "GrammarSymbol.h"

using std::vector;
using std::shared_ptr;

LR1Item::LR1Item(shared_ptr<GrammarRule> rule, shared_ptr<GrammarSymbol> lookahead) :
		rule { rule } {
	lookaheads.push_back(lookahead);
}

LR1Item::LR1Item(shared_ptr<GrammarRule> rule, vector<shared_ptr<GrammarSymbol>> lookaheads) :
		rule { rule },
		lookaheads { lookaheads } {
}

LR1Item::~LR1Item() {
}

void LR1Item::advance() {
	++visitedOffset;
	if (visitedOffset > rule->getProduction().size()) {
		throw std::out_of_range { "attempted to advance LR1Item past production size" };
	}
}

bool LR1Item::operator==(const LR1Item& rhs) const {
	return (this->rule == rhs.rule) && (this->visitedOffset == rhs.visitedOffset) && (this->lookaheads == rhs.lookaheads);
}

bool LR1Item::coresAreEqual(const LR1Item& that) const {
	return (this->rule == that.rule) && (this->visitedOffset == that.visitedOffset);
}

void LR1Item::mergeLookaheads(const std::vector<std::shared_ptr<GrammarSymbol>>& lookaheadsToMerge) {
	for (const auto& lookahead : lookaheadsToMerge)
	if (std::find(lookaheads.begin(), lookaheads.end(), lookahead) == lookaheads.end()) {
		lookaheads.push_back(lookahead);
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
	out << "[ " << *item.getDefiningSymbol() << " -> ";
	for (const auto& visitedSymbol : item.getVisited()) {
		out << *visitedSymbol << " ";
	}
	out << ". ";
	for (const auto& expectedSymbol : item.getExpected()) {
		out << *expectedSymbol << " ";
	}
	out << ", ";
	for (const auto& lookahead : item.getLookaheads()) {
		out << *lookahead << " ";
	}
	out << "]\n";
	return out;
}
