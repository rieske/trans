#include "parser/LR1Strategy.h"

#include <algorithm>
#include <iterator>

namespace parser {

LR1Strategy::~LR1Strategy() {
}

void LR1Strategy::computeCanonicalCollection(
        std::vector<std::vector<LR1Item>>& canonicalCollection,
        std::map<std::pair<std::size_t, std::string>, std::size_t>& computedGotos,
        const std::vector<GrammarSymbol>& grammarSymbols,
        const GoTo& goTo) const

{
    for (std::size_t i = 0; i < canonicalCollection.size(); ++i) { // for each set of items I in C
        for (const auto& X : grammarSymbols) { // and each grammar symbol X
            const auto& goto_I_X = goTo(canonicalCollection.at(i), X);
            if (!goto_I_X.empty()) { // such that goto(I, X) is not empty
                const auto& existingGotoIterator = std::find(canonicalCollection.begin(), canonicalCollection.end(), goto_I_X);
                if (existingGotoIterator == canonicalCollection.end()) { // and not in C
                    canonicalCollection.push_back(goto_I_X);
                    computedGotos[ { i, X.getDefinition() }] = canonicalCollection.size() - 1;
                } else {
                    computedGotos[ { i, X.getDefinition() }] = existingGotoIterator - canonicalCollection.begin();
                }
            }
        }
    }
}

}
