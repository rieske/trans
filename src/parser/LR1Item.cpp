#include "LR1Item.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "GrammarSymbol.h"

using std::vector;
using std::shared_ptr;

LR1Item::LR1Item(shared_ptr<GrammarSymbol> definingSymbol, GrammarRule productionRule, vector<shared_ptr<GrammarSymbol>> lookaheads) :
		definingSymbol { definingSymbol },
		production { productionRule.getProduction() },
		lookaheads { lookaheads } {
}

LR1Item::~LR1Item() {
}

void LR1Item::advance() {
	++visitedOffset;
	if (visitedOffset > production.size()) {
		throw std::out_of_range { "attempted to advance LR1Item past production size" };
	}
}

bool LR1Item::operator==(const LR1Item& rhs) const {
	return (this->definingSymbol == rhs.definingSymbol) && (this->production == rhs.production)
			&& (this->visitedOffset == rhs.visitedOffset) && (this->lookaheads == rhs.lookaheads);
}

bool LR1Item::coresAreEqual(const LR1Item& that) const {
	return (this->definingSymbol == that.definingSymbol) && (this->production == that.production)
			&& (this->visitedOffset == that.visitedOffset);
}

void LR1Item::mergeLookaheads(const std::vector<std::shared_ptr<GrammarSymbol>>& lookaheadsToMerge) {
	for (const auto& lookahead : lookaheadsToMerge)
		if (std::find(lookaheads.begin(), lookaheads.end(), lookahead) == lookaheads.end()) {
			lookaheads.push_back(lookahead);
		}
}

shared_ptr<GrammarSymbol> LR1Item::getDefiningSymbol() const {
	return definingSymbol;
}

vector<shared_ptr<GrammarSymbol>> LR1Item::getVisited() const {
	return vector<shared_ptr<GrammarSymbol>> { production.begin(), production.begin() + visitedOffset };
}

vector<shared_ptr<GrammarSymbol>> LR1Item::getExpected() const {
	return vector<shared_ptr<GrammarSymbol>> { production.begin() + visitedOffset, production.end() };
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
