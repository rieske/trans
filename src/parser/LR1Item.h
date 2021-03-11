#ifndef _ITEM_H_
#define _ITEM_H_

#include <ostream>
#include <vector>

#include "Production.h"
#include "parser/Grammar.h"

namespace parser {

// [ definingSymbol -> visited . expected, lookaheads ]

class LR1Item {
public:
    LR1Item(const Production& production, const std::vector<GrammarSymbol>& lookaheads);
    virtual ~LR1Item();

    LR1Item advance() const;
    bool mergeLookaheads(const std::vector<GrammarSymbol>& lookaheadsToMerge);

    const GrammarSymbol getDefiningSymbol() const;
    std::vector<GrammarSymbol> getVisited() const;
    bool hasUnvisitedSymbols() const;
    const GrammarSymbol& nextUnvisitedSymbol() const;
    std::vector<GrammarSymbol> getExpectedSymbols() const;
    std::vector<GrammarSymbol> getLookaheads() const;

    Production getProduction() const;

    bool coresAreEqual(const LR1Item& that) const;
    bool operator==(const LR1Item& rhs) const;

    std::string str(const Grammar& grammar) const;
private:
    const Production production;
    size_t visitedOffset { 0 };
    std::vector<GrammarSymbol> lookaheads;
};

} // namespace parser

#endif // _ITEM_H_
