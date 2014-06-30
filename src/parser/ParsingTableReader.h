#ifndef PARSINGTABLEREADER_H_
#define PARSINGTABLEREADER_H_

#include <stddef.h>
#include <fstream>
#include <string>
#include <utility>

class ParsingTableReader {
public:
	ParsingTableReader(std::string fileName);
	virtual ~ParsingTableReader();

	size_t readStateCount();
	void readDelimiter();
	std::string readSerializedAction();
	std::pair<bool, size_t> readGotoRecord();

private:
	std::ifstream parsingTableStream;
};

#endif /* PARSINGTABLEREADER_H_ */
