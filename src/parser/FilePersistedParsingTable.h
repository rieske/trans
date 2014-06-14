#ifndef PARSINGTABLEREADER_H_
#define PARSINGTABLEREADER_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "ParsingTable.h"

class Grammar;

class FilePersistedParsingTable: public ParsingTable {
public:
	FilePersistedParsingTable(std::string parsingTableFilename, const Grammar& grammar);
	virtual ~FilePersistedParsingTable();

private:
	void readTable(std::istream& parsingTableStream, const Grammar& grammar);
};

#endif /* PARSINGTABLEREADER_H_ */
