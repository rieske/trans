#include "LR1Item.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "GrammarSymbol.h"

using std::vector;
using std::shared_ptr;

LR1Item::LR1Item(shared_ptr<const GrammarSymbol> definingSymbol, size_t productionId, vector<shared_ptr<const GrammarSymbol>> lookaheads) :
		definingSymbol { definingSymbol },
		productionId { productionId },
		lookaheads { lookaheads } {
}

LR1Item::~LR1Item() {
}

void LR1Item::advance() {
	++visitedOffset;
	if (visitedOffset > definingSymbol->getProductions().at(productionId).size()) {
		throw std::out_of_range { "attempted to advance LR1Item past production size" };
	}
}

bool LR1Item::operator==(const LR1Item& rhs) const {
	return (this->definingSymbol == rhs.definingSymbol) && (this->productionId == rhs.productionId)
			&& (this->visitedOffset == rhs.visitedOffset) && (this->lookaheads == rhs.lookaheads);
}

bool LR1Item::coresAreEqual(const LR1Item& that) const {
	return (this->definingSymbol == that.definingSymbol) && (this->productionId == that.productionId)
			&& (this->visitedOffset == that.visitedOffset);
}

void LR1Item::mergeLookaheads(const std::vector<std::shared_ptr<const GrammarSymbol>>& lookaheadsToMerge) {
	for (const auto& lookahead : lookaheadsToMerge) {
		if (std::find(lookaheads.begin(), lookaheads.end(), lookahead) == lookaheads.end()) {
			lookaheads.push_back(lookahead);
		}
	}
}

shared_ptr<const GrammarSymbol> LR1Item::getDefiningSymbol() const {
	return definingSymbol;
}

vector<shared_ptr<const GrammarSymbol>> LR1Item::getVisited() const {
	Production production = definingSymbol->getProductions().at(productionId);
	return vector<shared_ptr<const GrammarSymbol>> { production.begin(), production.begin() + visitedOffset };
}

vector<shared_ptr<const GrammarSymbol>> LR1Item::getExpected() const {
	Production production = definingSymbol->getProductions().at(productionId);
	return vector<shared_ptr<const GrammarSymbol>> { production.begin() + visitedOffset, production.end() };
}

vector<shared_ptr<const GrammarSymbol>> LR1Item::getLookaheads() const {
	return lookaheads;
}

size_t LR1Item::getProductionId() const {
	return productionId;
}

Production LR1Item::getProduction() const {
	return definingSymbol->getProductions().at(productionId);
}

std::string LR1Item::productionStr() const {
	std::string ret = "";
	for (auto& symbol : getProduction()) {
		if (symbol->getName().length() && *symbol->getName().begin() != '<' && *symbol->getName().end() - 1 != '>') {
			ret += "'" + symbol->getName() + "'";
		} else {
			ret += symbol->getName();
		}
		ret += " ";
	}
	return ret.substr(0, ret.size() - 1);
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
