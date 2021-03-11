#include "BNFFileGrammar.h"

#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <map>

#include <iostream>

namespace {

const std::string TERMINAL_CONFIG_DELIMITER = "%%";
const char NONTERMINAL_START = '<';
const char NONTERMINAL_END = '>';
const char TERMINAL_START = '\'';
const char TERMINAL_END = '\'';

struct RuleStub {
    std::string resultName;
    std::vector<std::string> producedSymbolNames;
    std::size_t id;
};

} // namespace

namespace parser {

BNFFileGrammar::BNFFileGrammar(const std::string bnfFileName) {
    std::ifstream bnfInputStream { bnfFileName };
    if (!bnfInputStream.is_open()) {
        throw std::invalid_argument("Unable to open bnf file for reading: " + bnfFileName);
    }

    std::vector<std::string> symbolsBeingDefined;
    std::vector<RuleStub> rulesBeingDefined;

    std::string nonterminalName;
    std::vector<int> nonterminalBeingDefinedRuleIndexes;
    std::vector<std::string> producedSymbolNames;

    int nextSymbolId = 0;
    std::map<std::string, GrammarSymbol> definedSymbols;

    for (std::string bnfToken; bnfInputStream >> bnfToken && bnfToken != TERMINAL_CONFIG_DELIMITER;) {
        if (bnfToken.length() == 1) {
            switch (bnfToken.front()) {
                case '|': {
                    RuleStub rule { nonterminalName, producedSymbolNames, rulesBeingDefined.size() };
                    producedSymbolNames.clear();
                    nonterminalBeingDefinedRuleIndexes.push_back(rule.id);
                    rulesBeingDefined.push_back(rule);
                    break;
                }
                case ';': {
                    RuleStub rule { nonterminalName, producedSymbolNames, rulesBeingDefined.size() };
                    producedSymbolNames.clear();
                    nonterminalBeingDefinedRuleIndexes.push_back(rule.id);
                    rulesBeingDefined.push_back(rule);
                    definedSymbols.insert({nonterminalName, { nextSymbolId++, nonterminalBeingDefinedRuleIndexes }});
                    nonterminals.push_back(definedSymbols.at(nonterminalName));
                    nonterminalName.clear();
                    nonterminalBeingDefinedRuleIndexes.clear();
                    break;
                }
                case ':':
                    break;
                default:
                    throw std::runtime_error("Unrecognized control character in grammar configuration file: " + bnfToken);
            }
        } else if (!bnfToken.empty() && bnfToken.front() == NONTERMINAL_START && bnfToken.back() == NONTERMINAL_END) {
            const auto& nonterminalBeingDefinedIterator = std::find(symbolsBeingDefined.begin(), symbolsBeingDefined.end(), bnfToken);
            if (nonterminalBeingDefinedIterator == symbolsBeingDefined.end()) {
                symbolsBeingDefined.push_back(bnfToken);
            }
            if (!nonterminalName.empty()) {
                producedSymbolNames.push_back(bnfToken);
            } else {
                nonterminalName = bnfToken;
            }
        } else if (!bnfToken.empty() && bnfToken.front() == TERMINAL_START && bnfToken.back() == TERMINAL_END) {
            std::string symbolName { bnfToken.substr(1, bnfToken.size() - 2) };
            if (definedSymbols.find(symbolName) == definedSymbols.end()) {
                definedSymbols.insert({symbolName, { nextSymbolId++ }});
                terminals.push_back(definedSymbols.at(symbolName));
            }
            producedSymbolNames.push_back(symbolName);
        } else {
            throw std::runtime_error("Unrecognized token in grammar configuration file: " + bnfToken);
        }
    }

    for (const auto& ruleStub : rulesBeingDefined) {
        std::vector<GrammarSymbol> production;
        for (const auto& symbolName : ruleStub.producedSymbolNames) {
            production.push_back(definedSymbols.at(symbolName));
        }
        rules.push_back({ definedSymbols.at(ruleStub.resultName), production, ruleStub.id });
    }

    for (const auto& symbol : definedSymbols) {
        symbols.push_back(symbol.second);
        symbolIDs.insert({symbol.first, symbol.second.getId()});
    }

    terminals.push_back(getEndSymbol());
    rules.push_back({ getStartSymbol(), { nonterminals.front() }, rules.size() });
}

BNFFileGrammar::~BNFFileGrammar() {
}

std::vector<GrammarSymbol> BNFFileGrammar::getTerminals() const {
    return terminals;
}

std::vector<GrammarSymbol> BNFFileGrammar::getNonterminals() const {
    return nonterminals;
}

std::size_t BNFFileGrammar::ruleCount() const {
    return rules.size();
}

const Production& BNFFileGrammar::getRuleByIndex(int index) const {
    return rules.at(index);
}

std::vector<Production> BNFFileGrammar::getProductionsOfSymbol(const GrammarSymbol& symbol) const {
    std::vector<Production> productions;
    for (const auto& ruleIndex : symbol.getRuleIndexes()) {
        productions.push_back(rules.at(ruleIndex));
    }
    return productions;
}

void BNFFileGrammar::addSymbol(int id) {
    auto existingSymbolIterator = std::find_if(symbols.begin(), symbols.end(),
            [&id](const GrammarSymbol& terminal) {return terminal.getId() == id;});
    if (existingSymbolIterator == symbols.end()) {
        symbols.push_back(GrammarSymbol { id });
    }
}

} // namespace parser

