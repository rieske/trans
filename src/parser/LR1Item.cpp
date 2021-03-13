#include "LR1Item.h"

#include <algorithm>
#include <sstream>

namespace parser {

LR1Item::LR1Item(const Production& production, const std::vector<int>& lookaheads) :
        production { production },
        lookaheads { lookaheads }
{
}

LR1Item LR1Item::advance() const {
    LR1Item advancedItem { *this };
    ++advancedItem.visitedOffset;
    if (advancedItem.visitedOffset > static_cast<int>(production.size())) {
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

bool LR1Item::mergeLookaheads(const std::vector<int>& lookaheadsToMerge) {
    bool lookaheadsAdded { false };
    for (const auto& lookahead : lookaheadsToMerge) {
        if (std::find(lookaheads.begin(), lookaheads.end(), lookahead) == lookaheads.end()) {
            lookaheads.insert(std::lower_bound(lookaheads.begin(), lookaheads.end(), lookahead), lookahead);
            lookaheadsAdded = true;
        }
    }
    return lookaheadsAdded;
}

std::vector<int> LR1Item::getVisited() const {
    return {production.begin(), production.begin() + visitedOffset};
}

bool LR1Item::hasUnvisitedSymbols() const {
    return visitedOffset < static_cast<int>(production.size());
}

int LR1Item::nextUnvisitedSymbol() const {
    return *(production.begin() + visitedOffset);
}

std::vector<int> LR1Item::getExpectedSymbols() const {
    return {production.begin() + visitedOffset, production.end()};
}

const std::vector<int>& LR1Item::getLookaheads() const {
    return lookaheads;
}

Production LR1Item::getProduction() const {
    return production;
}

std::string LR1Item::str(const Grammar& grammar) const {
    std::stringstream out;
    out << "[ " << grammar.str(production.getDefiningSymbol()) << " -> ";
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

