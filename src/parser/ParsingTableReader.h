#ifndef PARSINGTABLEREADER_H_
#define PARSINGTABLEREADER_H_

#include <fstream>
#include <string>
#include <tuple>

#include "LookaheadActionTable.h"

namespace parser {

class ParsingTableReader {
public:
	ParsingTableReader(std::string fileName);

	size_t readStateCount();
	void readDelimiter();
	std::string readSerializedAction();
	std::tuple<parse_state, int, parse_state> readGotoRecord();
	bool endOfFile() const;

private:
	std::ifstream parsingTableStream;
};

} // namespace parser

#endif // PARSINGTABLEREADER_H_
