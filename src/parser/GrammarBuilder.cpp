#include "GrammarBuilder.h"

#include <algorithm>

namespace parser {

struct Rule {
    int nonterminalId;
    std::vector<int> producedSymbolIds;
    std::size_t id;
};

void GrammarBuilder::defineRule(std::string nonterminal, std::vector<std::string> production) {
    if (!nonterminalDefinitionExists(nonterminal)) {
        defineSymbol(nonterminal);
        nonterminalDefinitions.push_back(nonterminal);
    }
    productions.insert({nonterminal, production});
}

Grammar GrammarBuilder::build() {
    std::vector<GrammarSymbol> terminals;
    std::map<int, GrammarSymbol> allSymbols;
    std::map<int, std::vector<int>> nonterminalRules;
    std::map<int, std::vector<int>> ruleProductions;
    int ruleId {0};
    for (const auto& production: productions) {
        int nonterminalId = symbolIDs.at(production.first);
        auto& ruleIndexes = nonterminalRules.insert({nonterminalId, {}}).first->second;
        ruleIndexes.push_back(ruleId);
        for (const auto& producedSymbol: production.second) {
            if (!nonterminalDefinitionExists(producedSymbol) && !symbolExists(producedSymbol)) {
                GrammarSymbol terminal {defineSymbol(producedSymbol)};
                allSymbols.insert({terminal.getId(), terminal});
                terminals.push_back(terminal);
            }
            auto& productions = ruleProductions.insert({ruleId, {}}).first->second;
            productions.push_back(symbolIDs.at(producedSymbol));
        }

        ruleId++;
    }

    std::vector<GrammarSymbol> nonterminals;
    for (const auto& rule: nonterminalRules) {
        GrammarSymbol nonterminal {rule.first, rule.second};
        nonterminals.push_back(nonterminal);
        allSymbols.insert({nonterminal.getId(), nonterminal});
    }

    std::vector<Production> rules;
    for (const auto& nonterminal: nonterminals) {
        for (const auto& ruleId: nonterminal.getRuleIndexes()) {
            std::vector<GrammarSymbol> producedSymbols;
            for (const auto& symbolId: ruleProductions.at(ruleId)) {
                producedSymbols.push_back(allSymbols.at(symbolId));
            }
            rules.push_back({nonterminal.getId(), producedSymbols, static_cast<size_t>(ruleId)});
        }
    }

    return {symbolIDs, terminals, nonterminals, rules};
}

int GrammarBuilder::defineSymbol(std::string symbolName) {
    return symbolIDs.insert({symbolName, nextSymbolId++}).first->second;
}

bool GrammarBuilder::symbolExists(std::string symbolName) const {
    return symbolIDs.find(symbolName) != symbolIDs.end();
}

bool GrammarBuilder::nonterminalDefinitionExists(std::string nonterminal) const {
    return std::find(nonterminalDefinitions.begin(), nonterminalDefinitions.end(), nonterminal) != nonterminalDefinitions.end();
}

} // namespace parser

