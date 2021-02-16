#include "LALR1Strategy.h"

#include <algorithm>

namespace {

using namespace parser;

bool containsItemsWithSameCores(const std::vector<LR1Item>& existingItems, const std::vector<LR1Item>& itemsToMergeOrAdd) {
    if (existingItems.size() != itemsToMergeOrAdd.size()) {
        return false;
    }
    for (std::size_t j = 0; j < existingItems.size(); ++j) {
        if (!existingItems.at(j).coresAreEqual(itemsToMergeOrAdd.at(j))) {
            return false;
        }
    }
    return true;
}

} // namespace

namespace parser {

LALR1Strategy::~LALR1Strategy() {
}

void LALR1Strategy::computeCanonicalCollection(
        std::vector<std::vector<LR1Item> >& canonicalCollection,
        std::map<std::pair<std::size_t, std::string>, std::size_t>& computedGotos,
        const std::vector<GrammarSymbol>& grammarSymbols,
        const GoTo& goTo) const

{
    std::vector<int> modifiedStates;
    modifiedStates.push_back(0);
    while (!modifiedStates.empty()) {
        auto statesToRevisit = modifiedStates;
        modifiedStates.clear();
        for (auto state : statesToRevisit) {
            for (const auto& X : grammarSymbols) { // and each grammar symbol X
                const auto& goto_I_X = goTo(canonicalCollection.at(state), X);
                if (!goto_I_X.empty()) { // such that goto(I, X) is not empty
                    const auto& iteratorToExistingSetWithSameCores = std::find_if(canonicalCollection.begin(), canonicalCollection.end(),
                            [&goto_I_X] (const std::vector<LR1Item>& existingSet) {
                                return containsItemsWithSameCores(existingSet, goto_I_X);
                            });
                    if (iteratorToExistingSetWithSameCores == canonicalCollection.end()) { // and not in C
                        canonicalCollection.push_back(goto_I_X);
                        computedGotos[ { state, X.getDefinition() }] = canonicalCollection.size() - 1;
                        modifiedStates.push_back(canonicalCollection.size() - 1);
                    } else {
                        bool lookaheadsMerged { false };
                        for (std::size_t j = 0; j < iteratorToExistingSetWithSameCores->size(); ++j) {
                            lookaheadsMerged |= iteratorToExistingSetWithSameCores->at(j).mergeLookaheads(goto_I_X.at(j).getLookaheads());
                        }
                        auto stateIndex = iteratorToExistingSetWithSameCores - canonicalCollection.begin();
                        computedGotos[ { state, X.getDefinition() }] = stateIndex;
                        if (lookaheadsMerged) {
                            modifiedStates.push_back(stateIndex);
                        }
                    }
                }
            }
        }
    }
}

} // namespace parser

