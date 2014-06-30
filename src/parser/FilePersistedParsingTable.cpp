#include "FilePersistedParsingTable.h"

#include "Action.h"
#include "Grammar.h"
#include "GrammarSymbol.h"
#include "ParsingTableReader.h"

using std::string;

FilePersistedParsingTable::FilePersistedParsingTable(string parsingTableFilename, const Grammar* grammar) :
		ParsingTable { grammar } {

	ParsingTableReader tableReader { parsingTableFilename };

	parse_state stateCount = tableReader.readStateCount();
	tableReader.readDelimiter();

	for (parse_state stateNumber = 0; stateNumber < stateCount; ++stateNumber) {
		for (const auto& terminal : grammar->getTerminals()) {
			string serializedAction = tableReader.readSerializedAction();
			lookaheadActions[stateNumber][terminal->getSymbol()] = Action::deserialize(serializedAction, this);
		}
	}

	tableReader.readDelimiter();

	for (parse_state stateNumber = 0; stateNumber < stateCount; ++stateNumber) {
		for (const auto& nonterminal : grammar->getNonterminals()) {
			std::pair<bool, parse_state> gotoRecord = tableReader.readGotoRecord();
			if (gotoRecord.first) {
				gotoTable[stateNumber][nonterminal->getSymbol()] = gotoRecord.second;
			}
		}
	}
}

FilePersistedParsingTable::~FilePersistedParsingTable() {
}
