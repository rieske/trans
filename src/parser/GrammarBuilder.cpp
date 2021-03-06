#include "GrammarBuilder.h"

#include <vector>
#include <algorithm>

namespace parser {

void GrammarBuilder::defineRule(std::string nonterminal, std::vector<std::string> production) {
    if (!nonterminalDefinitionExists(nonterminal)) {
        defineSymbol(nonterminal);
        nonterminalDefinitions.push_back(nonterminal);
    }
    productions.insert({nonterminal, production});
}

Grammar GrammarBuilder::build() {
    std::vector<int> terminals;
    std::map<int, std::vector<int>> nonterminalRules;
    std::map<int, std::vector<int>> ruleProductions;
    int ruleId {0};
    for (const auto& production: productions) {
        int nonterminalId = symbolIDs.at(production.first);
        nonterminalRules.insert({nonterminalId, {}}).first->second.push_back(ruleId);
        for (const auto& producedSymbol: production.second) {
            if (!nonterminalDefinitionExists(producedSymbol) && !symbolExists(producedSymbol)) {
                terminals.push_back(defineSymbol(producedSymbol));
            }
            ruleProductions.insert({ruleId, {}}).first->second.push_back(symbolIDs.at(producedSymbol));
        }

        ruleId++;
    }

    std::vector<int> nonterminals;
    std::vector<Production> rules;
    for (const auto& nonterminal: nonterminalRules) {
        nonterminals.push_back(nonterminal.first);
        for (const auto& ruleId: nonterminal.second) {
            std::vector<int> producedSymbols;
            for (const auto& symbolId: ruleProductions.at(ruleId)) {
                producedSymbols.push_back(symbolId);
            }
            rules.push_back({nonterminal.first, producedSymbols, ruleId});
        }
    }
    std::sort(rules.begin(), rules.end(), [](const Production& p1, const Production& p2) -> bool {
        return p1.getId() < p2.getId();
    });

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

