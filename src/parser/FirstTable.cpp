#include "FirstTable.h"

#include <algorithm>
#include <sstream>

namespace parser {

FirstTable::FirstTable(const Grammar& grammar) {
    auto symbols = grammar.getNonterminalIDs();
    initializeTable(symbols, grammar);

    bool moreToAdd = true;
    while (moreToAdd) {
        moreToAdd = false;
        for (const auto& symbol : symbols) {
            for (auto& production : grammar.getProductionsOfSymbol(symbol)) {
                const auto& firstProductionSymbol = *production.begin();
                for (const auto& firstSymbol : firstTable.at(firstProductionSymbol)) {
                    moreToAdd |= addFirstSymbol(symbol, firstSymbol);
                }
            }
        }
    }
}

const std::vector<int> FirstTable::operator()(int symbolId) const {
    return firstTable.at(symbolId);
}

bool FirstTable::addFirstSymbol(int firstFor, int firstSymbol) {
    auto& firstSetForSymbol = firstTable.at(firstFor);
    if (std::find(firstSetForSymbol.begin(), firstSetForSymbol.end(), firstSymbol) == firstSetForSymbol.end()) {
        firstSetForSymbol.push_back(firstSymbol);
        return true;
    }
    return false;
}

void FirstTable::initializeTable(const std::vector<int>& symbols, const Grammar& grammar) {
    for (const auto& symbol : symbols) {
        firstTable.insert({symbol, {}});
        for (auto production : grammar.getProductionsOfSymbol(symbol)) {
            for (const auto& productionSymbol : production) {
                firstTable.insert({productionSymbol, {}});
                if (grammar.isTerminal(productionSymbol)) {
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

