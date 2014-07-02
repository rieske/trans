#ifndef _LR1PARSER_H_
#define _LR1PARSER_H_

#include <memory>

#include "Parser.h"

class ParsingTable;

class LR1Parser: public Parser {
public:
	LR1Parser(ParsingTable* parsingTable);
	virtual ~LR1Parser();

	void parse(Scanner& scanner, SemanticAnalyzer& semanticAnalyzer) override;
private:
	std::unique_ptr<ParsingTable> parsingTable;
};

#endif // _LR1PARSER_H_
