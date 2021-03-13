#include "CanonicalCollection.h"

#include "util/Logger.h"
#include "util/LogManager.h"

namespace {
static Logger& logger = LogManager::getComponentLogger(Component::PARSER);
} // namespace

namespace parser {

CanonicalCollection::CanonicalCollection(const FirstTable& firstTable, const Grammar& grammar, const CanonicalCollectionStrategy& strategy) :
        firstTable { firstTable }
{
    std::vector<int> grammarSymbols = grammar.getNonterminalIDs();
    std::vector<int> terminals = grammar.getTerminalIDs();
    grammarSymbols.insert(std::end(grammarSymbols), std::begin(terminals), std::end(terminals));

    LR1Item initialItem { grammar.getRuleByIndex(grammar.ruleCount() - 1), { grammar.getEndSymbol().getId() } };
    std::vector<LR1Item> initialSet { initialItem };
    Closure closure { firstTable, &grammar };
    closure(initialSet);
    canonicalCollection.push_back(initialSet);
    GoTo goTo { closure };

    strategy.computeCanonicalCollection(canonicalCollection, computedGotos, grammarSymbols, goTo);

    logCollection(grammar);
}

size_t CanonicalCollection::stateCount() const noexcept {
    return canonicalCollection.size();
}

const std::vector<LR1Item>& CanonicalCollection::setOfItemsAtState(size_t state) const {
    return canonicalCollection.at(state);
}

std::size_t CanonicalCollection::goTo(std::size_t stateFrom, int symbol) const {
    return computedGotos.at( { stateFrom, symbol });
}

void CanonicalCollection::logCollection(const Grammar& grammar) const {
    logger << "\n*********************";
    logger << "\nCanonical collection:\n";
    logger << "*********************\n";
    int setNo { 0 };
    for (const auto& setOfItems : canonicalCollection) {
        logger << "Set " << setNo++ << ":\n";
        for (const auto& item : setOfItems) {
            logger << item.str(grammar);
        }
        logger << "\n";
    }

    logger << "Computed GOTOs:\n";
    for (const auto& computedGoto : computedGotos) {
        logger << computedGoto.first.first << "\t" << computedGoto.first.second << "\t" << computedGoto.second << "\n";
    }
}

} // namespace parser

