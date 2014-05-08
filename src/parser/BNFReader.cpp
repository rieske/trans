#include "BNFReader.h"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <algorithm>

#include "GrammarRule.h"
#include "NonterminalSymbol.h"
#include "TerminalSymbol.h"

using std::string;
using std::ifstream;
using std::shared_ptr;

const string TERMINAL_CONFIG_DELIMITER = "\%\%";

const char NONTERMINAL_START = '<';
const char NONTERMINAL_END = '>';
const char TERMINAL_START = '\'';
const char TERMINAL_END = '\'';

BNFReader::BNFReader(const string bnfFileName) {
	ifstream bnfInputStream { bnfFileName };
	if (!bnfInputStream.is_open()) {
		throw std::invalid_argument("Unable to open bnf file for reading: " + bnfFileName);
	}
	string bnfToken;
	GrammarRule* rule { nullptr };
	shared_ptr<GrammarSymbol> ruleLeftSide;
	int ruleId { 1 };
	while (bnfInputStream >> bnfToken && bnfToken != TERMINAL_CONFIG_DELIMITER) {
		if (bnfToken.length() == 1) {
			switch (bnfToken.at(0)) {
			case '|':
				rules.push_back(shared_ptr<GrammarRule> { rule });
				rule = new GrammarRule(ruleLeftSide, ruleId++);
				break;
			case ';':
				rules.push_back(shared_ptr<GrammarRule> { rule });
				rule = nullptr;
				break;
			case ':':
				break;
			default:
				throw std::runtime_error("Unrecognized control character in grammar configuration file: " + bnfToken);
			}
		} else if (!bnfToken.empty() && bnfToken.at(0) == NONTERMINAL_START && bnfToken.at(bnfToken.length() - 1) == NONTERMINAL_END) {
			shared_ptr<GrammarSymbol> nonterminal = addNonterminal(bnfToken);
			if (rule) {
				rule->addRight(nonterminal);
			} else {
				ruleLeftSide = nonterminal;
				rule = new GrammarRule(ruleLeftSide, ruleId++);
			}
		} else if (!bnfToken.empty() && *bnfToken.begin() == TERMINAL_START && *(bnfToken.end() - 1) == TERMINAL_END) {
			shared_ptr<GrammarSymbol> terminal = addTerminal(bnfToken);
			rule->addRight(terminal);
		} else {
			throw std::runtime_error("Unrecognized token in grammar configuration file: " + bnfToken);
		}
	}
	int terminalId;
	string terminalName;
	while (bnfInputStream >> terminalId >> terminalName) {
		idToTerminalMappingTable[terminalId] = findTerminalByName(terminalName);
	}
	bnfInputStream.close();
}

BNFReader::~BNFReader() {
}

shared_ptr<GrammarSymbol> BNFReader::findTerminalByName(const string& name) const {
	auto terminalIterator = std::find_if(terminals.begin(), terminals.end(),
			[&name](const shared_ptr<GrammarSymbol>& terminal) {return terminal->getName() == name;});
	if (terminalIterator == terminals.end()) {
		throw std::invalid_argument("Terminal not used in grammar: " + name);
	}
	return *terminalIterator;
}

shared_ptr<GrammarSymbol> BNFReader::addTerminal(const string& name) {
	auto existingTerminalIterator = std::find_if(terminals.begin(), terminals.end(),
			[&name](const shared_ptr<GrammarSymbol>& terminal) {return terminal->getName() == name;});
	if (existingTerminalIterator != terminals.end()) {
		return *existingTerminalIterator;
	}
	shared_ptr<GrammarSymbol> newTerminal { new TerminalSymbol { name } };
	terminals.push_back(newTerminal);
	return newTerminal;
}

shared_ptr<GrammarSymbol> BNFReader::addNonterminal(const string& name) {
	auto existingNonterminalIterator = std::find_if(nonterminals.begin(), nonterminals.end(),
			[&name](const shared_ptr<GrammarSymbol>& nonterminal) {return nonterminal->getName() == name;});
	if (existingNonterminalIterator != nonterminals.end()) {
		return *existingNonterminalIterator;
	}
	shared_ptr<GrammarSymbol> newNonterminal { new NonterminalSymbol { name } };
	nonterminals.push_back(newNonterminal);
	return newNonterminal;
}

const std::vector<std::shared_ptr<GrammarRule>> BNFReader::getRules() const {
	return rules;
}

std::vector<std::shared_ptr<GrammarSymbol>> BNFReader::getTerminals() const {
	return terminals;
}

std::vector<std::shared_ptr<GrammarSymbol>> BNFReader::getNonterminals() const {
	return nonterminals;
}

std::map<int, std::shared_ptr<GrammarSymbol>> BNFReader::getIdToTerminalMappingTable() const {
	return idToTerminalMappingTable;
}
