#include "FilePersistedParsingTable.h"

#include <fstream>
#include <stdexcept>

#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "Action.h"
#include "Grammar.h"
#include "GrammarSymbol.h"

using std::string;
using std::ifstream;
using std::istream;

const static string CONFIGURATION_DELIMITER = "\%\%";

static Logger& logger = LogManager::getComponentLogger(Component::PARSER);

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

void FilePersistedParsingTable::readDelimiter(istream& table) const {
	string delim;
	table >> delim;
	if (delim != CONFIGURATION_DELIMITER) {
		throw std::runtime_error("error in parsing table configuration file: " + CONFIGURATION_DELIMITER + " delimiter expected");
	}
	std::getline(table, delim);
}

void FilePersistedParsingTable::readTable(istream& table, const Grammar& grammar) {
	parse_state stateCount;
	table >> stateCount;

	readDelimiter(table);

	for (parse_state stateNumber = 0; stateNumber < stateCount; ++stateNumber) {
		for (const auto& terminal : grammar.terminals) {
			string serializedAction;
			if (!std::getline(table, serializedAction)) {
				throw std::runtime_error { "error reading parsing table action" };
			}
			terminalActionTables[stateNumber][terminal->getName()] = Action::deserialize(serializedAction, grammar, &gotoTable);
		}
	}

	readDelimiter(table);

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
