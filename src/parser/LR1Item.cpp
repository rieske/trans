#include "LR1Item.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "GrammarSymbol.h"

using std::vector;
using std::string;

namespace parser {

LR1Item::LR1Item(const GrammarSymbol* definingGrammarSymbol, size_t productionNumber, const vector<const GrammarSymbol*>& lookaheads) :
		definingSymbol { definingGrammarSymbol->getDefinition() },
		productionNumber { productionNumber },
		production { definingGrammarSymbol->getProductions().at(productionNumber) },
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
	return coresAreEqual(rhs) && (this->lookaheads == rhs.lookaheads);
}

bool LR1Item::coresAreEqual(const LR1Item& that) const {
	return (this->definingSymbol == that.definingSymbol) && (this->productionNumber == that.productionNumber)
			&& (this->visitedOffset == that.visitedOffset);
}

void LR1Item::mergeLookaheads(const std::vector<const GrammarSymbol*>& lookaheadsToMerge) {
	for (const auto& lookahead : lookaheadsToMerge) {
		if (std::find(lookaheads.begin(), lookaheads.end(), lookahead) == lookaheads.end()) {
			lookaheads.push_back(lookahead);
		}
	}
}

string LR1Item::getDefiningSymbol() const {
	return definingSymbol;
}

vector<const GrammarSymbol*> LR1Item::getVisited() const {
	return {production.begin(), production.begin() + visitedOffset};
}

vector<const GrammarSymbol*> LR1Item::getExpectedSymbols() const {
	return {production.begin() + visitedOffset, production.end()};
}

vector<const GrammarSymbol*> LR1Item::getLookaheads() const {
	return lookaheads;
}

size_t LR1Item::getProductionNumber() const {
	return productionNumber;
}

Production LR1Item::getProduction() const {
	return production;
}

std::ostream& operator<<(std::ostream& out, const LR1Item& item) {
	out << "[ " << item.getDefiningSymbol() << " -> ";
	for (const auto& visitedSymbol : item.getVisited()) {
		out << *visitedSymbol << " ";
	}
	out << ". ";
	for (const auto& expectedSymbol : item.getExpectedSymbols()) {
		out << *expectedSymbol << " ";
	}
	out << ", ";
	for (const auto& lookahead : item.getLookaheads()) {
		out << *lookahead << " ";
	}
	out << "]\n";
	return out;
}

}
