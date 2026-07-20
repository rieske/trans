#ifndef FILE_PERSISTED_PARSING_TABLE_H_
#define FILE_PERSISTED_PARSING_TABLE_H_

#include <string>

#include "ParsingTable.h"

namespace parser {

class FilePersistedParsingTable: public ParsingTable {
public:
	FilePersistedParsingTable(std::string parsingTableFilename, const Grammar* grammar);
	~FilePersistedParsingTable() override = default;
};

} // namespace parser

#endif // FILE_PERSISTED_PARSING_TABLE_H_
