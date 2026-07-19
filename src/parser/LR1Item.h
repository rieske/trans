#ifndef _ITEM_H_
#define _ITEM_H_

#include "Production.h"
#include "parser/Grammar.h"

#include <cstdint>
#include <vector>

namespace parser {

// LR(1) item: [ A -> α • β, lookaheads ] with dense terminal bitsets.
class LR1Item {
public:
    using LookaheadSet = parser::LookaheadSet;

    explicit LR1Item(const Production& production, LookaheadSet lookaheads);
    LR1Item(const Production& production, const std::vector<int>& lookaheadSymbolIds, const Grammar& grammar);

    LR1Item advance() const;
    bool mergeLookaheads(const LookaheadSet& more);

    bool hasUnvisitedSymbols() const;
    int nextUnvisitedSymbol() const;
    bool hasSymbolsAfterNext() const;
    int symbolAfterNext() const;

    std::vector<int> getVisited() const;
    std::vector<int> getExpectedSymbols() const;
    std::vector<int> getLookaheads(const Grammar& grammar) const;
    const LookaheadSet& lookaheads() const noexcept;
    bool hasLookahead(int symbolId, const Grammar& grammar) const;

    const Production& getProduction() const;
    std::uint64_t coreKey() const noexcept;
    bool operator==(const LR1Item& rhs) const;

    std::string str(const Grammar& grammar) const;

private:
    const Production* production;
    int visitedOffset { 0 };
    LookaheadSet lookaheadBits;
};

} // namespace parser

#endif
