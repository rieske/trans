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

    std::map<std::string, GrammarSymbol> symbolsBeingDefined { };
    std::vector<Production> rulesBeingDefined { };

    GrammarSymbol* nonterminalBeingDefined { nullptr };
    vector<GrammarSymbol> producedSymbols { };
    for (string bnfToken; bnfInputStream >> bnfToken && bnfToken != TERMINAL_CONFIG_DELIMITER;) {
        if (bnfToken.length() == 1) {
            switch (bnfToken.front()) {
            case '|': {
                Production production { producedSymbols, rulesBeingDefined.size() };
                nonterminalBeingDefined->addRuleIndex(production.getId());
                rulesBeingDefined.push_back(production);
                producedSymbols.clear();
                break;
            }
            case ';': {
                Production production { producedSymbols, rulesBeingDefined.size() };
                nonterminalBeingDefined->addRuleIndex(production.getId());
                rulesBeingDefined.push_back(production);
                producedSymbols.clear();
                // FIXME: make grammar symbol immutable and add it here
                symbols.push_back(*nonterminalBeingDefined);
                nonterminalBeingDefined = nullptr;
                break;
            }
            case ':':
                break;
            default:
                throw std::runtime_error("Unrecognized control character in grammar configuration file: " + bnfToken);
            }
        } else if (!bnfToken.empty() && bnfToken.front() == NONTERMINAL_START && bnfToken.back() == NONTERMINAL_END) {
            if (symbolsBeingDefined.find(bnfToken) == symbolsBeingDefined.end()) {
                symbolsBeingDefined.insert(std::make_pair(bnfToken, GrammarSymbol { bnfToken }));
            }
            auto& nonterminal = symbolsBeingDefined.at(bnfToken);

            if (nonterminalBeingDefined) {
                producedSymbols.push_back(nonterminal);
            } else {
                nonterminalBeingDefined = &nonterminal;
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
            if (symbolsBeingDefined.find(symbol.getDefinition()) != symbolsBeingDefined.end()) {
                production.push_back(symbolsBeingDefined.at(symbol.getDefinition()));
            } else {
                production.push_back(symbol);
            }
        }
        rules.push_back( { production, ruleStub.getId() });
    }

    terminals.push_back(getEndSymbol());
    Production production { { nonterminals.front() }, rules.size() };
    startSymbol.addRuleIndex(production.getId());
    rules.push_back(production);
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
