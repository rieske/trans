#include "BNFReader.h"

#include <fstream>
#include <iterator>
#include <stdexcept>
//#include <utility>

#include "GrammarRule.h"
#include "NonterminalSymbol.h"
#include "TerminalSymbol.h"

using std::string;
using std::ifstream;
using std::shared_ptr;

const string TERMINAL_CONFIG_DELIMITER = "\%\%";

BNFReader::BNFReader(const string bnfFileName) {
	ifstream bnfInputStream { bnfFileName };
	if (!bnfInputStream.is_open()) {
		throw std::invalid_argument("Unable to open bnf file for reading: " + bnfFileName);
	}
	string bnfToken;
	GrammarRule *rule { nullptr };
	shared_ptr<GrammarSymbol> left;
	int ruleId { 1 };
	while (bnfInputStream >> bnfToken) {
		if (bnfToken == TERMINAL_CONFIG_DELIMITER) {
			int terminalId;
			string terminalName;
			while (bnfInputStream >> terminalId >> terminalName) {
				idToTerminalMappingTable[terminalId] = findTerminalByName(terminalName);
			}
		} else if (bnfToken.length() == 1) {
			switch (bnfToken.at(0)) {
			case '|':
				rules.push_back(shared_ptr<GrammarRule> { rule });
				rule = new GrammarRule(left, ruleId++);
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
		} else if (!bnfToken.empty() && bnfToken.at(0) == '<' && bnfToken.at(bnfToken.length() - 1) == '>') {
			shared_ptr<GrammarSymbol> nonterminal = addNonterminal(bnfToken);
			if (rule) {
				rule->addRight(nonterminal);
			} else {
				left = nonterminal;
				rule = new GrammarRule(left, ruleId++);
			}
		} else if (!bnfToken.empty() && *bnfToken.begin() == '\'' && *(bnfToken.end() - 1) == '\'') {
			shared_ptr<GrammarSymbol> terminal = addTerminal(bnfToken);
			rule->addRight(terminal);
		}
	}
	bnfInputStream.close();
}

BNFReader::~BNFReader() {
}

shared_ptr<GrammarSymbol> BNFReader::findTerminalByName(const string& name) const {
	for (shared_ptr<GrammarSymbol> terminal : terminals) {
		if (terminal->getName() == name) {
			return terminal;
		}
	}
	throw std::invalid_argument("Terminal not used in grammar: " + name);
}

shared_ptr<GrammarSymbol> BNFReader::addTerminal(const string& name) {
	for (shared_ptr<GrammarSymbol> terminal : terminals) {
		if (terminal->getName() == name) {
			return terminal;
		}
	}
	shared_ptr<GrammarSymbol> newTerminal { new TerminalSymbol { name } };
	terminals.push_back(newTerminal);
	return newTerminal;
}

shared_ptr<GrammarSymbol> BNFReader::addNonterminal(const string& name) {
	for (shared_ptr<GrammarSymbol> nonterminal : nonterminals) {
		if (nonterminal->getName() == name) {
			return nonterminal;
		}
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
