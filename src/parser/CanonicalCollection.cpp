#include "CanonicalCollection.h"

#include <algorithm>
#include <iostream>
#include <iterator>

#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "Closure.h"
#include "GoTo.h"
#include "Grammar.h"
#include "GrammarSymbol.h"

using std::vector;

namespace parser {

static Logger& logger = LogManager::getComponentLogger(Component::PARSER);

CanonicalCollection::CanonicalCollection(const FirstTable& firstTable, const Grammar& grammar) :
        firstTable { firstTable }
{

    vector<const GrammarSymbol*> grammarSymbols;
    for (const auto& terminal : grammar.getTerminals()) {
        grammarSymbols.push_back(terminal);
    }
    for (const auto& nonterminal : grammar.getNonterminals()) {
        grammarSymbols.push_back(nonterminal);
    }

    LR1Item initialItem { grammar.getStartSymbol(), 0, { grammar.getEndSymbol() } };
    vector<LR1Item> initialSet { initialItem };
    Closure closure { firstTable };
    closure(initialSet);
    canonicalCollection.push_back(initialSet);
    GoTo goTo { closure };

    for (std::size_t i = 0; i < canonicalCollection.size(); ++i) { // for each set of items I in C
        if ((canonicalCollection.size() % 1000) == 0) {
            std::cout << canonicalCollection.size() << " " << i << std::endl;
        }
        for (const auto& X : grammarSymbols) { // and each grammar symbol X
            const auto& goto_I_X = goTo(canonicalCollection.at(i), X);
            if (!goto_I_X.empty()) { // such that goto(I, X) is not empty
                const auto& existingGotoIterator = std::find(canonicalCollection.begin(), canonicalCollection.end(), goto_I_X);
                if (existingGotoIterator == canonicalCollection.end()) { // and not in C
                    canonicalCollection.push_back(goto_I_X);
                    computedGotos[ { i, X->getDefinition() }] = canonicalCollection.size() - 1;
                } else {
                    computedGotos[ { i, X->getDefinition() }] = existingGotoIterator - canonicalCollection.begin();
                }
            }
        }
    }

    logCollection();
}

CanonicalCollection::~CanonicalCollection() {
}

size_t CanonicalCollection::stateCount() const noexcept {
    return canonicalCollection.size();
}

const std::vector<LR1Item>& CanonicalCollection::setOfItemsAtState(size_t state) const {
    return canonicalCollection.at(state);
}

std::size_t CanonicalCollection::goTo(std::size_t stateFrom, std::string symbol) const {
    return computedGotos.at( { stateFrom, symbol });
}

void CanonicalCollection::logCollection() const {
    logger << "\n*********************";
    logger << "\nCanonical collection:\n";
    logger << "*********************\n";
    int setNo { 0 };
    for (const auto& setOfItems : canonicalCollection) {
        logger << "Set " << setNo++ << ":\n";
        for (const auto& item : setOfItems) {
            logger << item;
        }
        logger << "\n";
    }
}

}
