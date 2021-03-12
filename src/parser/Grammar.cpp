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
    symbolIDs.insert({"<__start__>", startSymbol.getId()});
    symbolIDs.insert({"'$end$'", endSymbol.getId()});
}

Grammar::~Grammar() {
}

std::size_t Grammar::ruleCount() const {
    return rules.size();
}

const Production& Grammar::getRuleByIndex(int index) const {
    return rules.at(index);
}

std::vector<Production> Grammar::getProductionsOfSymbol(const GrammarSymbol& symbol) const {
    std::vector<Production> productions;
    for (const auto& ruleIndex : symbol.getRuleIndexes()) {
        productions.push_back(rules.at(ruleIndex));
    }
    return productions;
}

std::vector<GrammarSymbol> Grammar::getTerminals() const {
    return terminals;
}

std::vector<GrammarSymbol> Grammar::getNonterminals() const {
    return nonterminals;
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

std::string Grammar::str(const GrammarSymbol& symbol) const {
    return getSymbolById(symbol.getId());
}

std::ostream& operator<<(std::ostream& out, const Grammar& grammar) {
    out << "\nTerminals:\n";
    for (auto& terminal : grammar.getTerminals()) {
        out << terminal.getId() << ":" << grammar.str(terminal) << "\n";
    }
    out << "\nNonterminals:\n";
    for (auto& nonterminal : grammar.getNonterminals()) {
        out << nonterminal.getId() << ":" << grammar.str(nonterminal) << "\n";
    }
    out << "\nRules total: " << grammar.ruleCount() << "\n";
    return out;
}

} // namespace parser

