#ifndef FILE_PERSISTED_PARSING_TABLE_H_
#define FILE_PERSISTED_PARSING_TABLE_H_

#include <string>

#include "ParsingTable.h"

class Grammar;

class FilePersistedParsingTable: public ParsingTable {
public:
	FilePersistedParsingTable(std::string parsingTableFilename, const Grammar* grammar);
	virtual ~FilePersistedParsingTable();
};

#endif /* FILE_PERSISTED_PARSING_TABLE_H_ */
