#include "FilePersistedParsingTable.h"

namespace parser {

FilePersistedParsingTable::FilePersistedParsingTable(std::string parsingTableFilename, const Grammar* grammar) :
        ParsingTable(grammar)
{
    loadFromFile(parsingTableFilename);
}

} // namespace parser
