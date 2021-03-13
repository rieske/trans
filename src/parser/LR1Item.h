#ifndef _ITEM_H_
#define _ITEM_H_

#include "Production.h"
#include "parser/Grammar.h"

namespace parser {

// [ definingSymbol -> visited . expected, lookaheads ]

class LR1Item {
public:
    LR1Item(const Production& production, const std::vector<int>& lookaheads);

    LR1Item advance() const;
    bool mergeLookaheads(const std::vector<int>& lookaheadsToMerge);

    const GrammarSymbol getDefiningSymbol() const;
    std::vector<int> getVisited() const;
    bool hasUnvisitedSymbols() const;
    int nextUnvisitedSymbol() const;
    std::vector<int> getExpectedSymbols() const;
    std::vector<int> getLookaheads() const;

    Production getProduction() const;

    bool coresAreEqual(const LR1Item& that) const;
    bool operator==(const LR1Item& rhs) const;

    std::string str(const Grammar& grammar) const;
private:
    const Production production;
    size_t visitedOffset { 0 };
    std::vector<int> lookaheads;
};

} // namespace parser

#endif // _ITEM_H_
