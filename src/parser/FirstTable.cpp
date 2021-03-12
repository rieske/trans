#include "FirstTable.h"

#include <algorithm>
#include <sstream>

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
                for (const auto& firstSymbol : firstTable.at(firstProductionSymbol.getId())) {
                    moreToAdd |= addFirstSymbol(symbol, firstSymbol);
                }
            }
        }
    }
}

const std::vector<GrammarSymbol> FirstTable::operator()(const GrammarSymbol& symbol) const {
    return firstTable.at(symbol.getId());
}

bool FirstTable::addFirstSymbol(const GrammarSymbol& firstFor, const GrammarSymbol& firstSymbol) {
    auto& firstSetForSymbol = firstTable.at(firstFor.getId());
    if (std::find(firstSetForSymbol.begin(), firstSetForSymbol.end(), firstSymbol) == firstSetForSymbol.end()) {
        firstSetForSymbol.push_back(firstSymbol);
        return true;
    }
    return false;
}

void FirstTable::initializeTable(const std::vector<GrammarSymbol>& symbols, const Grammar& grammar) {
    for (const auto& symbol : symbols) {
        if (firstTable.find(symbol.getId()) == firstTable.end()) {
            firstTable[symbol.getId()] = std::vector<GrammarSymbol> { };
        }
        for (auto production : grammar.getProductionsOfSymbol(symbol)) {
            for (const auto& productionSymbol : production) {
                if (firstTable.find(productionSymbol.getId()) == firstTable.end()) {
                    firstTable[productionSymbol.getId()] = std::vector<GrammarSymbol> { };
                }
                if (productionSymbol.isTerminal()) {
                    addFirstSymbol(productionSymbol, productionSymbol);
                }
            }
        }
    }
}

std::string FirstTable::str(const Grammar& grammar) const{
    std::stringstream s;
    for (const auto& symbolFirstSet : firstTable) {
        s << "FIRST(" << grammar.str(symbolFirstSet.first) << "): ";
        for (const auto& firstSymbol : symbolFirstSet.second) {
            s << grammar.str(firstSymbol) << " ";
        }
        s << "\n";
    }
    return s.str();
}

} // namespace parser

