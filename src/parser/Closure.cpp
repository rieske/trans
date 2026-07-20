#include "Closure.h"

#include <algorithm>

namespace parser {

Closure::Closure(const FirstTable& first, const Grammar* grammar) :
        first { first },
        grammar { grammar }
{
}

void Closure::operator()(std::vector<LR1Item>& items) const {
    bool more = true;
    while (more) {
        more = false;
        for (size_t i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            if (item.hasUnvisitedSymbols() && !grammar->isTerminal(item.nextUnvisitedSymbol())) {
                const auto& nextExpectedNonterminal = item.nextUnvisitedSymbol();
                const LR1Item::LookaheadSet propagated = item.hasSymbolsAfterNext()
                        ? first.firstBits(item.symbolAfterNext())
                        : item.lookaheads();
                for (const auto& production : grammar->getProductionsOfSymbol(nextExpectedNonterminal)) {
                    LR1Item newItem { production, propagated };
                    const auto& existingItemIt = std::find_if(items.begin(), items.end(),
                            [&newItem] (const LR1Item& existingItem) {
                                return existingItem.coreKey() == newItem.coreKey();
                            });
                    if (existingItemIt == items.end()) {
                        items.push_back(newItem);
                        more = true;
                    } else if (existingItemIt->mergeLookaheads(newItem.lookaheads())) {
                        more = true;
                    }
                }
            }
        }
    }
}

} // namespace parser
