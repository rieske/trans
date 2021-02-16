#include "Closure.h"

#include <algorithm>

namespace parser {

Closure::Closure(const FirstTable& first, const Grammar* grammar) :
        first { first },
        grammar { grammar }
{
}

Closure::~Closure() {
}

void Closure::operator()(std::vector<LR1Item>& items) const {
    bool more = true;
    while (more) {
        more = false;
        for (size_t i = 0; i < items.size(); ++i) {
            const auto item = items.at(i);
            if (item.hasUnvisitedSymbols() && item.nextUnvisitedSymbol().isNonterminal()) { // [ A -> u.Bv, a ] (expected[0] == B)
                const auto& nextExpectedNonterminal = item.nextUnvisitedSymbol();
                const auto& expectedSymbols = item.getExpectedSymbols();
                std::vector<GrammarSymbol> firstForNextSymbol {
                        (expectedSymbols.size() > 1) ? first(expectedSymbols.at(1)) : item.getLookaheads() };
                for (const auto& ruleIndex : nextExpectedNonterminal.getRuleIndexes()) {
                    LR1Item newItem { grammar->getRuleByIndex(ruleIndex), firstForNextSymbol };
                    const auto& existingItemIt = std::find_if(items.begin(), items.end(),
                            [&newItem] (const LR1Item& existingItem) {
                                return existingItem.coresAreEqual(newItem);
                            });
                    if (existingItemIt == items.end()) {
                        items.push_back(newItem);
                        more = true;
                    } else {
                        existingItemIt->mergeLookaheads(newItem.getLookaheads());
                    }
                }
            }
        }
    }
}

} // namespace parser

