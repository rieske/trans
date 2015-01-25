#include "LR1Item.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "GrammarSymbol.h"

using std::vector;
using std::string;

namespace parser {

LR1Item::LR1Item(const Production& production, const vector<GrammarSymbol>& lookaheads) :
        production { production },
        lookaheads { lookaheads }
{
    std::sort(this->lookaheads.begin(), this->lookaheads.end());
}

LR1Item::~LR1Item() {
}

LR1Item LR1Item::advance() const {
    LR1Item advancedItem { *this };
    ++advancedItem.visitedOffset;
    if (advancedItem.visitedOffset > production.size()) {
        throw std::out_of_range { "attempted to advance LR1Item past production size" };
    }
    return advancedItem;
}

bool LR1Item::operator==(const LR1Item& rhs) const {
    return coresAreEqual(rhs) && (this->lookaheads == rhs.lookaheads);
}

bool LR1Item::coresAreEqual(const LR1Item& that) const {
    return this->production.getId() == that.production.getId() && this->visitedOffset == that.visitedOffset;
}

bool LR1Item::mergeLookaheads(const std::vector<GrammarSymbol>& lookaheadsToMerge) {
    bool lookaheadsAdded { false };
    for (const auto& lookahead : lookaheadsToMerge) {
        if (std::find(lookaheads.begin(), lookaheads.end(), lookahead) == lookaheads.end()) {
            lookaheads.push_back(lookahead);
            lookaheadsAdded = true;
        }
    }
    if (lookaheadsAdded) {
        std::sort(lookaheads.begin(), lookaheads.end());
    }
    return lookaheadsAdded;
}

const GrammarSymbol LR1Item::getDefiningSymbol() const {
    return production.getDefiningSymbol();
}

vector<GrammarSymbol> LR1Item::getVisited() const {
    return {production.begin(), production.begin() + visitedOffset};
}

bool LR1Item::hasUnvisitedSymbols() const {
    return visitedOffset < production.size();
}

const GrammarSymbol& LR1Item::nextUnvisitedSymbol() const {
    return *(production.begin() + visitedOffset);
}

vector<GrammarSymbol> LR1Item::getExpectedSymbols() const {
    return {production.begin() + visitedOffset, production.end()};
}

vector<GrammarSymbol> LR1Item::getLookaheads() const {
    return lookaheads;
}

Production LR1Item::getProduction() const {
    return production;
}

std::ostream& operator<<(std::ostream& out, const LR1Item& item) {
    out << "[ " << item.getDefiningSymbol().getDefinition() << " -> ";
    for (const auto& visitedSymbol : item.getVisited()) {
        out << visitedSymbol << " ";
    }
    out << ". ";
    for (const auto& expectedSymbol : item.getExpectedSymbols()) {
        out << expectedSymbol << " ";
    }
    out << ", ";
    for (const auto& lookahead : item.getLookaheads()) {
        out << lookahead << " ";
    }
    out << "]\n";
    return out;
}

}
