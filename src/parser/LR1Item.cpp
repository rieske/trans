#include "LR1Item.h"

#include <algorithm>
#include <sstream>

namespace parser {

LR1Item::LR1Item(const Production& production, const std::vector<GrammarSymbol>& lookaheads) :
        production { production },
        lookaheads { lookaheads }
{
    std::sort(this->lookaheads.begin(), this->lookaheads.end());
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

std::vector<GrammarSymbol> LR1Item::getVisited() const {
    return {production.begin(), production.begin() + visitedOffset};
}

bool LR1Item::hasUnvisitedSymbols() const {
    return visitedOffset < production.size();
}

const GrammarSymbol& LR1Item::nextUnvisitedSymbol() const {
    return *(production.begin() + visitedOffset);
}

std::vector<GrammarSymbol> LR1Item::getExpectedSymbols() const {
    return {production.begin() + visitedOffset, production.end()};
}

std::vector<GrammarSymbol> LR1Item::getLookaheads() const {
    return lookaheads;
}

Production LR1Item::getProduction() const {
    return production;
}

std::string LR1Item::str(const Grammar& grammar) const {
    std::stringstream out;
    out << "[ " << grammar.str(getDefiningSymbol()) << " -> ";
    for (const auto& visitedSymbol : getVisited()) {
        out << grammar.str(visitedSymbol) << " ";
    }
    out << ". ";
    for (const auto& expectedSymbol : getExpectedSymbols()) {
        out << grammar.str(expectedSymbol) << " ";
    }
    out << ", ";
    for (const auto& lookahead : getLookaheads()) {
        out << grammar.str(lookahead) << " ";
    }
    out << "]\n";
    return out.str();
}

} // namespace parser

