#ifndef _LR1PARSER_H_
#define _LR1PARSER_H_

#include <memory>

#include "Parser.h"

namespace parser {

class ParsingTable;

class LR1Parser: public Parser {
public:
	LR1Parser(ParsingTable* parsingTable);
	virtual ~LR1Parser();

	std::unique_ptr<SyntaxTree> parse(scanner::Scanner& scanner, SyntaxTreeBuilder& syntaxTreeBuilder) override;
private:
	std::unique_ptr<ParsingTable> parsingTable;
};

} // namespace parser

#endif // _LR1PARSER_H_
