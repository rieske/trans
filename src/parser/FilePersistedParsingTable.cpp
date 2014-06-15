#include "FilePersistedParsingTable.h"

#include <fstream>
#include <map>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "AcceptAction.h"
#include "ErrorAction.h"
#include "Grammar.h"
#include "GrammarSymbol.h"
#include "LR1Item.h"
#include "ReduceAction.h"
#include "ShiftAction.h"

using std::string;
using std::ifstream;
using std::istream;
using std::map;
using std::unordered_map;
using std::array;
using std::unique_ptr;

static Logger& logger = LogManager::getComponentLogger(Component::PARSER);

const char SHIFT_ACTION = 's';
const char REDUCE_ACTION = 'r';
const char ERROR_ACTION = 'e';
const char ACCEPT_ACTION = 'a';
const string CONFIGURATION_DELIMITER = "\%\%";

FilePersistedParsingTable::FilePersistedParsingTable(string parsingTableFilename, const Grammar& grammar) {
	logger << grammar;

	ifstream parsingTableStream { parsingTableFilename };
	if (!parsingTableStream.is_open()) {
		throw std::runtime_error("could not open parsing table configuration file for reading. Filename: " + parsingTableFilename);
	}
	readTable(parsingTableStream, grammar);
	parsingTableStream.close();
}

FilePersistedParsingTable::~FilePersistedParsingTable() {
}

void FilePersistedParsingTable::readTable(istream& table, const Grammar& grammar) {
	parse_state stateCount;
	table >> stateCount;

	string delim;
	table >> delim;
	if (delim != CONFIGURATION_DELIMITER) {
		throw std::runtime_error("error in parsing table configuration file: " + CONFIGURATION_DELIMITER + " delimiter expected");
	}

	for (parse_state stateNumber = 0; stateNumber < stateCount; ++stateNumber) {
		for (const auto& terminal : grammar.terminals) {
			char type;
			table >> type;
			parse_state state;
			table >> state;
			switch (type) {
			case SHIFT_ACTION:
				terminalActionTables[stateNumber][terminal->getName()] = unique_ptr<Action> { new ShiftAction(state) };
				break;
			case REDUCE_ACTION: {
				size_t nonterminalId;
				size_t productionId;
				table >> nonterminalId >> productionId;
				terminalActionTables[stateNumber][terminal->getName()] = unique_ptr<Action> { new ReduceAction(
						grammar.getReductionById(nonterminalId, productionId), &gotoTable) };
				break;
			}
			case ACCEPT_ACTION:
				terminalActionTables[stateNumber][grammar.endSymbol->getName()] = unique_ptr<Action> { new AcceptAction() };
				break;
			case ERROR_ACTION: {
				string forge;
				string expected;
				table >> forge >> expected;
				terminalActionTables[stateNumber][terminal->getName()] = unique_ptr<Action> { new ErrorAction(state, forge, expected) };
				break;
			}
			default:
				throw std::runtime_error("Error in parsing table configuration file: invalid action type: " + type);
			}
		}
	}

	table >> delim;
	if (delim != CONFIGURATION_DELIMITER) {
		throw std::runtime_error("error in parsing table configuration file: " + CONFIGURATION_DELIMITER + " delimiter expected");
	}

	// pildom goto lentelę
	for (parse_state stateNumber = 0; stateNumber < stateCount; ++stateNumber) {
		for (const auto& nonterminal : grammar.nonterminals) {
			char type;
			parse_state state;
			table >> type >> state;
			if (type == '0') {
				continue;
			}
			gotoTable[stateNumber][nonterminal] = state;
		}
	}

}
