#include "BNFFileGrammar.h"

#include "GrammarSymbol.h"
#include "Grammar.h"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <algorithm>
#include <map>

using std::string;
using std::ifstream;
using std::unique_ptr;
using std::vector;

namespace {

const std::string TERMINAL_CONFIG_DELIMITER = "\%\%";
const char NONTERMINAL_START = '<';
const char NONTERMINAL_END = '>';
const char TERMINAL_START = '\'';
const char TERMINAL_END = '\'';

}

namespace parser {

BNFFileGrammar::BNFFileGrammar(const string bnfFileName) {
    ifstream bnfInputStream { bnfFileName };
    if (!bnfInputStream.is_open()) {
        throw std::invalid_argument("Unable to open bnf file for reading: " + bnfFileName);
    }

    std::vector<std::string> symbolsBeingDefined;
    std::vector<Production> rulesBeingDefined;

    std::string nonterminalBeingDefined;
    std::vector<std::size_t> nonterminalBeingDefinedRuleIndexes;
    std::map<std::string, GrammarSymbol> definedNonterminals;
    vector<GrammarSymbol> producedSymbols;
    for (string bnfToken; bnfInputStream >> bnfToken && bnfToken != TERMINAL_CONFIG_DELIMITER;) {
        if (bnfToken.length() == 1) {
            switch (bnfToken.front()) {
            case '|': {
                Production production { producedSymbols, rulesBeingDefined.size() };
                producedSymbols.clear();
                nonterminalBeingDefinedRuleIndexes.push_back(production.getId());
                rulesBeingDefined.push_back(production);
                break;
            }
            case ';': {
                Production production { producedSymbols, rulesBeingDefined.size() };
                producedSymbols.clear();
                nonterminalBeingDefinedRuleIndexes.push_back(production.getId());
                rulesBeingDefined.push_back(production);
                symbols.push_back( { nonterminalBeingDefined, nonterminalBeingDefinedRuleIndexes });
                definedNonterminals.insert(std::make_pair(nonterminalBeingDefined, symbols.back()));
                nonterminalBeingDefined.clear();
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
            if (!nonterminalBeingDefined.empty()) {
                producedSymbols.push_back( { bnfToken });
            } else {
                nonterminalBeingDefined = bnfToken;
            }
        } else if (!bnfToken.empty() && bnfToken.front() == TERMINAL_START && bnfToken.back() == TERMINAL_END) {
            const auto& terminal = addSymbol(bnfToken.substr(1, bnfToken.size() - 2));
            producedSymbols.push_back(terminal);
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
        for (const auto& symbol : ruleStub) {
            if (definedNonterminals.find(symbol.getDefinition()) != definedNonterminals.end()) {
                production.push_back(definedNonterminals.at(symbol.getDefinition()));
            } else {
                production.push_back(symbol);
            }
        }
        rules.push_back( { production, ruleStub.getId() });
    }

    terminals.push_back(getEndSymbol());
    Production production { { nonterminals.front() }, rules.size() };
    rules.push_back(production);
    startSymbol = GrammarSymbol { "<__start__>", { production.getId() } };
}

BNFFileGrammar::~BNFFileGrammar() {
}

vector<GrammarSymbol> BNFFileGrammar::getTerminals() const {
    return terminals;
}

vector<GrammarSymbol> BNFFileGrammar::getNonterminals() const {
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

GrammarSymbol& BNFFileGrammar::addSymbol(const string& name) {
    auto existingSymbolIterator = std::find_if(symbols.begin(), symbols.end(),
            [&name](const GrammarSymbol& terminal) {return terminal.getDefinition() == name;});
    if (existingSymbolIterator != symbols.end()) {
        return *existingSymbolIterator;
    }
    symbols.push_back(GrammarSymbol { name });
    return symbols.back();
}

}
