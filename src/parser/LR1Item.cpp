#include "LR1Item.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace parser {

LR1Item::LR1Item(const Production& production, LookaheadSet lookaheads) :
        production { &production },
        lookaheadBits { lookaheads }
{
}

LR1Item::LR1Item(const Production& production, const std::vector<int>& lookaheadSymbolIds, const Grammar& grammar) :
        production { &production },
        lookaheadBits { grammar.toLookaheadBits(lookaheadSymbolIds) }
{
}

LR1Item LR1Item::advance() const {
    LR1Item next { *this };
    ++next.visitedOffset;
    if (next.visitedOffset > static_cast<int>(production->size())) {
        throw std::out_of_range { "attempted to advance LR1Item past production size" };
    }
    return next;
}

bool LR1Item::mergeLookaheads(const LookaheadSet& more) {
    const LookaheadSet before = lookaheadBits;
    lookaheadBits |= more;
    return lookaheadBits != before;
}

bool LR1Item::hasUnvisitedSymbols() const {
    return visitedOffset < static_cast<int>(production->size());
}

int LR1Item::nextUnvisitedSymbol() const {
    return *(production->begin() + visitedOffset);
}

bool LR1Item::hasSymbolsAfterNext() const {
    return visitedOffset + 1 < static_cast<int>(production->size());
}

int LR1Item::symbolAfterNext() const {
    return *(production->begin() + visitedOffset + 1);
}

std::vector<int> LR1Item::getVisited() const {
    return { production->begin(), production->begin() + visitedOffset };
}

std::vector<int> LR1Item::getExpectedSymbols() const {
    return { production->begin() + visitedOffset, production->end() };
}

std::vector<int> LR1Item::getLookaheads(const Grammar& grammar) const {
    auto ids = grammar.toTerminalIds(lookaheadBits);
    std::sort(ids.begin(), ids.end());
    return ids;
}

const LR1Item::LookaheadSet& LR1Item::lookaheads() const noexcept {
    return lookaheadBits;
}

bool LR1Item::hasLookahead(int symbolId, const Grammar& grammar) const {
    return lookaheadBits.test(static_cast<std::size_t>(grammar.terminalBit(symbolId)));
}

const Production& LR1Item::getProduction() const {
    return *production;
}

std::uint64_t LR1Item::coreKey() const noexcept {
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(production->getId())) << 32)
            | static_cast<std::uint32_t>(visitedOffset);
}

bool LR1Item::operator==(const LR1Item& rhs) const {
    return coreKey() == rhs.coreKey() && lookaheadBits == rhs.lookaheadBits;
}

std::string LR1Item::str(const Grammar& grammar) const {
    std::stringstream out;
    out << "[ " << grammar.str(production->getDefiningSymbol()) << " -> ";
    for (const auto& s : getVisited()) {
        out << grammar.str(s) << " ";
    }
    out << ". ";
    for (const auto& s : getExpectedSymbols()) {
        out << grammar.str(s) << " ";
    }
    out << ", ";
    for (const auto& s : getLookaheads(grammar)) {
        out << grammar.str(s) << " ";
    }
    out << "]\n";
    return out.str();
}

} // namespace parser
