#include "Grammar.h"

#include <algorithm>
#include <sstream>

namespace parser {

Grammar::Grammar(std::map<std::string, int> symbolIDs,
        std::vector<GrammarSymbol> terminals,
        std::vector<GrammarSymbol> nonterminals,
        std::vector<Production> rules):
    symbolIDs{symbolIDs},
    terminals{terminals},
    nonterminals{nonterminals},
    rules{rules},
    startSymbol { 1000 },
    endSymbol { -1 }
{
    this->symbolIDs.insert({"<__start__>", startSymbol.getId()});
    this->symbolIDs.insert({"'$end$'", endSymbol.getId()});
    this->terminals.push_back(endSymbol);
    this->rules.push_back({ startSymbol.getId(), { nonterminals.front().getId() }, static_cast<int>(rules.size()) });

    for (const auto& terminal : this->terminals) {
        terminalIDs.insert(terminal.getId());
    }

    std::vector<Production> productions;
    for (const auto& production: this->rules) {
        symbolProductions.insert({production.getDefiningSymbol(), {}}).first->second.push_back(production);
    }

    for (const auto& nonterminal: nonterminals) {
        nonterminalIDs.push_back(nonterminal.getId());
    }
}

std::size_t Grammar::ruleCount() const {
    return rules.size();
}

const Production& Grammar::getRuleByIndex(int index) const {
    return rules.at(index);
}

const std::vector<Production>& Grammar::getProductionsOfSymbol(int symbolId) const {
    return symbolProductions.at(symbolId);
}

std::vector<GrammarSymbol> Grammar::getTerminals() const {
    return terminals;
}

std::vector<int> Grammar::getNonterminalIDs() const {
    return nonterminalIDs;
}

const GrammarSymbol& Grammar::getStartSymbol() const {
    return startSymbol;
}

const GrammarSymbol& Grammar::getEndSymbol() const {
    return endSymbol;
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

bool Grammar::isTerminal(int symbolId) const {
    return terminalIDs.find(symbolId) != terminalIDs.end();
}

std::string Grammar::str(int symbolId) const {
    return getSymbolById(symbolId);
}

std::string Grammar::str(const Production& production) const {
    std::stringstream s;
    s << str(production.getDefiningSymbol()) << " -> ";
    for (const auto& symbol: production) {
        s << str(symbol) << " ";
    }
    auto productionStr = s.str();
    return productionStr.substr(0, productionStr.length()-1);
}

std::ostream& operator<<(std::ostream& out, const Grammar& grammar) {
    out << "\nTerminals:\n";
    for (auto& terminal : grammar.getTerminals()) {
        out << terminal.getId() << ":" << grammar.str(terminal.getId()) << "\n";
    }
    out << "\nNonterminals:\n";
    for (auto& nonterminal : grammar.getNonterminalIDs()) {
        out << nonterminal << ":" << grammar.str(nonterminal) << "\n";
    }
    out << "\nRules total: " << grammar.ruleCount() << "\n";
    return out;
}

} // namespace parser

