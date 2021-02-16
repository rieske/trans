#include "FilePersistedParsingTable.h"

#include "Action.h"
#include "ParsingTableReader.h"

namespace parser {

FilePersistedParsingTable::FilePersistedParsingTable(std::string parsingTableFilename, const Grammar* p_grammar) :
        ParsingTable(p_grammar)
{
    ParsingTableReader tableReader { parsingTableFilename };

    parse_state stateCount = tableReader.readStateCount();
    tableReader.readDelimiter();

    for (parse_state stateNumber = 0; stateNumber < stateCount; ++stateNumber) {
        for (const auto& terminal : grammar->getTerminals()) {
            std::string serializedAction = tableReader.readSerializedAction();
            lookaheadActionTable.addAction(stateNumber, terminal.getDefinition(), Action::deserialize(serializedAction, *this, *grammar));
        }
    }

    tableReader.readDelimiter();

    while(!tableReader.endOfFile()) {
        std::tuple<parse_state, std::string, parse_state> gotoRecord = tableReader.readGotoRecord();
        gotoTable[std::get<0>(gotoRecord)][std::get<1>(gotoRecord)] = std::get<2>(gotoRecord);
    }
}

FilePersistedParsingTable::~FilePersistedParsingTable() {
}

} // namespace parser

