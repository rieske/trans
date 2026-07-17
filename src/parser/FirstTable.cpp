#include "FirstTable.h"

#include <algorithm>
#include <sstream>
#include <vector>

namespace parser {

FirstTable::FirstTable(const Grammar& grammar) :
        grammar_ { &grammar }
{
    initialize(grammar);

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto nonterminal : grammar.getNonterminalIDs()) {
            for (const auto& production : grammar.getProductionsOfSymbol(nonterminal)) {
                const int firstSymbol = *production.begin();
                auto& dest = bitsBySymbol[nonterminal];
                const auto before = dest;
                dest |= bitsBySymbol.at(firstSymbol);
                changed = changed || (dest != before);
            }
        }
    }
}

std::vector<int> FirstTable::operator()(int symbolId) const {
    return grammar_->toTerminalIds(bitsBySymbol.at(symbolId));
}

const LookaheadSet& FirstTable::firstBits(int symbolId) const {
    return bitsBySymbol.at(symbolId);
}

void FirstTable::addTerminal(int symbol, int terminal) {
    bitsBySymbol[symbol].set(static_cast<std::size_t>(grammar_->terminalBit(terminal)));
}

void FirstTable::initialize(const Grammar& grammar) {
    for (const auto nonterminal : grammar.getNonterminalIDs()) {
        bitsBySymbol.emplace(nonterminal, LookaheadSet {});
        for (const auto& production : grammar.getProductionsOfSymbol(nonterminal)) {
            for (const int symbol : production) {
                bitsBySymbol.emplace(symbol, LookaheadSet {});
                if (grammar.isTerminal(symbol)) {
                    addTerminal(symbol, symbol);
                }
            }
        }
    }
}

std::string FirstTable::str(const Grammar& grammar) const {
    std::vector<int> symbols;
    symbols.reserve(bitsBySymbol.size());
    for (const auto& entry : bitsBySymbol) {
        symbols.push_back(entry.first);
    }
    std::sort(symbols.begin(), symbols.end());

    std::stringstream s;
    for (const int symbol : symbols) {
        s << "FIRST(" << grammar.str(symbol) << "): ";
        for (const int t : grammar.toTerminalIds(bitsBySymbol.at(symbol))) {
            s << grammar.str(t) << " ";
        }
        s << "\n";
    }
    return s.str();
}

} // namespace parser
