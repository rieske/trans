#include "FirstTable.h"

#include <algorithm>

namespace parser {

FirstTable::FirstTable(const Grammar& grammar) {
    const auto& symbols = grammar.getNonterminals();
    initializeTable(symbols, grammar);

    bool moreToAdd = true;
    while (moreToAdd) {
        moreToAdd = false;
        for (const auto& symbol : symbols) {
            for (auto& production : grammar.getProductionsOfSymbol(symbol)) {
                const auto& firstProductionSymbol = *production.begin();
                for (const auto& firstSymbol : firstTable.at(firstProductionSymbol.getDefinition())) {
                    moreToAdd |= addFirstSymbol(symbol, firstSymbol);
                }
            }
        }
    }
}

const std::vector<GrammarSymbol> FirstTable::operator()(const GrammarSymbol& symbol) const {
    return firstTable.at(symbol.getDefinition());
}

bool FirstTable::addFirstSymbol(const GrammarSymbol& firstFor, const GrammarSymbol& firstSymbol) {
    auto& firstSetForSymbol = firstTable.at(firstFor.getDefinition());
    if (std::find(firstSetForSymbol.begin(), firstSetForSymbol.end(), firstSymbol) == firstSetForSymbol.end()) {
        firstSetForSymbol.push_back(firstSymbol);
        return true;
    }
    return false;
}

void FirstTable::initializeTable(const std::vector<GrammarSymbol>& symbols, const Grammar& grammar) {
    for (const auto& symbol : symbols) {
        if (firstTable.find(symbol.getDefinition()) == firstTable.end()) {
            firstTable[symbol.getDefinition()] = std::vector<GrammarSymbol> { };
        }
        for (auto production : grammar.getProductionsOfSymbol(symbol)) {
            for (const auto& productionSymbol : production) {
                if (firstTable.find(productionSymbol.getDefinition()) == firstTable.end()) {
                    firstTable[productionSymbol.getDefinition()] = std::vector<GrammarSymbol> { };
                }
                if (productionSymbol.isTerminal()) {
                    addFirstSymbol(productionSymbol, productionSymbol);
                }
            }
        }
    }
}

std::ostream& operator<<(std::ostream& ostream, const FirstTable& firstTable) {
    for (const auto& symbolFirstSet : firstTable.firstTable) {
        ostream << "FIRST(" << symbolFirstSet.first << "):\t";
        for (const auto& firstSymbol : symbolFirstSet.second) {
            ostream << firstSymbol << " ";
        }
        ostream << "\n";
    }
    return ostream;
}

} // namespace parser

