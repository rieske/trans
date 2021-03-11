#include "Grammar.h"

#include <algorithm>

namespace parser {

Grammar::Grammar():
    startSymbol { 1000 },
    endSymbol { -1 }
{
    symbolIDs.insert({"<__start__>", startSymbol.getId()});
    symbolIDs.insert({"'$end$'", endSymbol.getId()});
}

Grammar::~Grammar() {
}

const GrammarSymbol& Grammar::getStartSymbol() const {
    return startSymbol;
}

const GrammarSymbol& Grammar::getEndSymbol() const {
    return endSymbol;
}


std::ostream& operator<<(std::ostream& out, const Grammar& grammar) {
    out << "\nTerminals:\n";
    for (auto& terminal : grammar.getTerminals()) {
        out << terminal << "\n";
    }
    out << "\nNonterminals:\n";
    for (auto& nonterminal : grammar.getNonterminals()) {
        out << nonterminal << "\n";
    }
    return out;
}

std::string Grammar::getSymbolById(int symbolId) const {
    auto symbolIt = std::find_if(symbolIDs.begin(), symbolIDs.end(), [&](const std::pair<std::string, int>& pair) {
        return pair.second == symbolId;
    });
    return symbolIt->first;
}

int Grammar::symbolId(std::string definition) const {
    return symbolIDs.at(definition);
}

std::string Grammar::str(const GrammarSymbol& symbol) const {
    return getSymbolById(symbol.getId());
}

int Grammar::createOrGetSymbolId(std::string definition) {
    auto it = symbolIDs.find(definition);
    if (it != symbolIDs.end()) {
        return it->second;
    }
    int id = symbolIDs.size();
    symbolIDs.insert({definition, id});
    return id;
}

} // namespace parser

