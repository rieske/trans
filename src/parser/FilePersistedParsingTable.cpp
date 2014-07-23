#include "FilePersistedParsingTable.h"

#include "Action.h"
#include "Grammar.h"
#include "GrammarSymbol.h"
#include "ParsingTableReader.h"

using std::string;

namespace parser {

FilePersistedParsingTable::FilePersistedParsingTable(string parsingTableFilename, const Grammar* p_grammar) :
		ParsingTable(p_grammar) {
	ParsingTableReader tableReader { parsingTableFilename };

	parse_state stateCount = tableReader.readStateCount();
	tableReader.readDelimiter();

	for (parse_state stateNumber = 0; stateNumber < stateCount; ++stateNumber) {
		for (const auto& terminal : grammar->getTerminals()) {
			string serializedAction = tableReader.readSerializedAction();
			lookaheadActionTable.addAction(stateNumber, terminal->getDefinition(), Action::deserialize(serializedAction, *this, *grammar));
		}
	}

	tableReader.readDelimiter();

	for (parse_state stateNumber = 0; stateNumber < stateCount; ++stateNumber) {
		for (const auto& nonterminal : grammar->getNonterminals()) {
			std::pair<bool, parse_state> gotoRecord = tableReader.readGotoRecord();
			if (gotoRecord.first) {
				gotoTable[stateNumber][nonterminal->getDefinition()] = gotoRecord.second;
			}
		}
	}
}

FilePersistedParsingTable::~FilePersistedParsingTable() {
}

}
