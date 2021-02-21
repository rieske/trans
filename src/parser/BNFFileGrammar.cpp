#include "BNFFileGrammar.h"

#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <map>

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
    std::vector<std::size_t> nonterminalBeingDefinedRuleIndexes;
    std::map<std::string, GrammarSymbol> definedNonterminals;
    std::vector<std::string> producedSymbolNames;
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
                symbols.push_back( { nonterminalName, nonterminalBeingDefinedRuleIndexes });
                definedNonterminals.insert(std::make_pair(nonterminalName, symbols.back()));
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
            addSymbol(symbolName);
            producedSymbolNames.push_back(symbolName);
        } else {
            throw std::runtime_error("Unrecognized token in grammar configuration file: " + bnfToken);
        }
    }

    for (const auto& symbol : symbols) {
        if (symbol.isTerminal()) {
            terminals.push_back(symbol);
        } else {
            nonterminals.push_back(symbol);
        }
    }

    for (const auto& ruleStub : rulesBeingDefined) {
        std::vector<GrammarSymbol> production;
        for (const auto& symbolName : ruleStub.producedSymbolNames) {
            if (definedNonterminals.find(symbolName) != definedNonterminals.end()) {
                production.push_back(definedNonterminals.at(symbolName));
            } else {
                production.push_back( { symbolName });
            }
        }
        rules.push_back( { definedNonterminals.at(ruleStub.resultName), production, ruleStub.id });
    }

    terminals.push_back(getEndSymbol());
    startSymbol = GrammarSymbol { "<__start__>", { rules.size() } };
    Production production { startSymbol, { nonterminals.front() }, rules.size() };
    rules.push_back(production);
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

GrammarSymbol& BNFFileGrammar::addSymbol(const std::string& name) {
    auto existingSymbolIterator = std::find_if(symbols.begin(), symbols.end(),
            [&name](const GrammarSymbol& terminal) {return terminal.getDefinition() == name;});
    if (existingSymbolIterator != symbols.end()) {
        return *existingSymbolIterator;
    }
    symbols.push_back(GrammarSymbol { name });
    return symbols.back();
}

} // namespace parser

