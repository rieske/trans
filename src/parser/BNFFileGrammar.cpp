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

const string TERMINAL_CONFIG_DELIMITER = "\%\%";

const char NONTERMINAL_START = '<';
const char NONTERMINAL_END = '>';
const char TERMINAL_START = '\'';
const char TERMINAL_END = '\'';

BNFFileGrammar::BNFFileGrammar(const string bnfFileName) {
	ifstream bnfInputStream { bnfFileName };
	if (!bnfInputStream.is_open()) {
		throw std::invalid_argument("Unable to open bnf file for reading: " + bnfFileName);
	}

	GrammarSymbol* nonterminalBeingDefined { nullptr };
	Production production;
	for (string bnfToken; bnfInputStream >> bnfToken && bnfToken != TERMINAL_CONFIG_DELIMITER;) {
		if (bnfToken.length() == 1) {
			switch (bnfToken.at(0)) {
			case '|':
				nonterminalBeingDefined->addProduction(production);
				production.clear();
				break;
			case ';':
				nonterminalBeingDefined->addProduction(production);
				production.clear();
				nonterminalBeingDefined = nullptr;
				break;
			case ':':
				break;
			default:
				throw std::runtime_error("Unrecognized control character in grammar configuration file: " + bnfToken);
			}
		} else if (!bnfToken.empty() && bnfToken.at(0) == NONTERMINAL_START && bnfToken.at(bnfToken.length() - 1) == NONTERMINAL_END) {
			GrammarSymbol* nonterminal = addSymbol(bnfToken);
			if (nonterminalBeingDefined) {
				production.push_back(nonterminal);
			} else {
				nonterminalBeingDefined = nonterminal;
			}
		} else if (!bnfToken.empty() && *bnfToken.begin() == TERMINAL_START && *(bnfToken.end() - 1) == TERMINAL_END) {
			const GrammarSymbol* terminal = addSymbol(bnfToken.substr(1, bnfToken.size() - 2));
			production.push_back(terminal);
		} else {
			throw std::runtime_error("Unrecognized token in grammar configuration file: " + bnfToken);
		}
	}
	bnfInputStream.close();

	for (const auto& symbol : symbols) {
		if (symbol->isTerminal()) {
			terminals.push_back(symbol.get());
		} else {
			nonterminals.push_back(symbol.get());
		}
	}

	startSymbol->addProduction( { nonterminals.at(0) });
	this->terminals.push_back(getEndSymbol());
}

BNFFileGrammar::~BNFFileGrammar() {
}

vector<const GrammarSymbol*> BNFFileGrammar::getTerminals() const {
	return terminals;
}

vector<const GrammarSymbol*> BNFFileGrammar::getNonterminals() const {
	return nonterminals;
}

GrammarSymbol* BNFFileGrammar::addSymbol(const string& name) {
	auto existingSymbolIterator = std::find_if(symbols.begin(), symbols.end(),
			[&name](const unique_ptr<GrammarSymbol>& terminal) {return terminal->getName() == name;});
	if (existingSymbolIterator != symbols.end()) {
		return existingSymbolIterator->get();
	}
	symbols.push_back(unique_ptr<GrammarSymbol> { new GrammarSymbol(name) });
	return (symbols.end() - 1)->get();
}
