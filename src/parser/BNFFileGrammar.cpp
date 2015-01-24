#include "BNFFileGrammar.h"

#include "GrammarSymbol.h"
#include "Grammar.h"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <algorithm>

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

    GrammarSymbol* nonterminalBeingDefined { nullptr };
    vector<const GrammarSymbol*> producedSymbols;
    for (string bnfToken; bnfInputStream >> bnfToken && bnfToken != TERMINAL_CONFIG_DELIMITER;) {
        if (bnfToken.length() == 1) {
            switch (bnfToken.front()) {
            case '|': {
                Production production { producedSymbols, rules.size() };
                nonterminalBeingDefined->addRuleIndex(production.getId());
                rules.push_back(production);
                producedSymbols.clear();
                break;
            }
            case ';': {
                Production production { producedSymbols, rules.size() };
                nonterminalBeingDefined->addRuleIndex(production.getId());
                rules.push_back(production);
                producedSymbols.clear();
                // FIXME: make grammar symbol immutable and add it here
                nonterminalBeingDefined = nullptr;
                break;
            }
            case ':':
                break;
            default:
                throw std::runtime_error("Unrecognized control character in grammar configuration file: " + bnfToken);
            }
        } else if (!bnfToken.empty() && bnfToken.front() == NONTERMINAL_START && bnfToken.at(bnfToken.length() - 1) == NONTERMINAL_END) {
            GrammarSymbol* nonterminal = addSymbol(bnfToken);
            if (nonterminalBeingDefined) {
                producedSymbols.push_back(nonterminal);
            } else {
                nonterminalBeingDefined = nonterminal;
            }
        } else if (!bnfToken.empty() && *bnfToken.begin() == TERMINAL_START && *(bnfToken.end() - 1) == TERMINAL_END) {
            const GrammarSymbol* terminal = addSymbol(bnfToken.substr(1, bnfToken.size() - 2));
            producedSymbols.push_back(terminal);
        } else {
            throw std::runtime_error("Unrecognized token in grammar configuration file: " + bnfToken);
        }
    }

    for (const auto& symbol : symbols) {
        if (symbol->isTerminal()) {
            terminals.push_back(symbol.get());
        } else {
            nonterminals.push_back(symbol.get());
        }
    }

    terminals.push_back(getEndSymbol());
    Production production { { nonterminals.front() }, rules.size() };
    startSymbol->addRuleIndex(production.getId());
    rules.push_back(production);
}

BNFFileGrammar::~BNFFileGrammar() {
}

vector<const GrammarSymbol*> BNFFileGrammar::getTerminals() const {
    return terminals;
}

vector<const GrammarSymbol*> BNFFileGrammar::getNonterminals() const {
    return nonterminals;
}

std::size_t BNFFileGrammar::ruleCount() const {
    return rules.size();
}

const Production& BNFFileGrammar::getRuleByIndex(int index) const {
    return rules.at(index);
}

std::vector<Production> BNFFileGrammar::getProductionsOfSymbol(const GrammarSymbol* symbol) const {
    std::vector<Production> productions;
    for (const auto& ruleIndex : symbol->getRuleIndexes()) {
        productions.push_back(rules.at(ruleIndex));
    }
    return productions;
}

GrammarSymbol* BNFFileGrammar::addSymbol(const string& name) {
    auto existingSymbolIterator = std::find_if(symbols.begin(), symbols.end(),
            [&name](const unique_ptr<GrammarSymbol>& terminal) {return terminal->getDefinition() == name;});
    if (existingSymbolIterator != symbols.end()) {
        return existingSymbolIterator->get();
    }
    symbols.push_back(unique_ptr<GrammarSymbol> { new GrammarSymbol(name) });
    return (symbols.end() - 1)->get();
}

}
