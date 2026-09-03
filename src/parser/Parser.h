#ifndef PARSER_H_
#define PARSER_H_

#include "parser/SyntaxTreeBuilder.h"
#include "scanner/Scanner.h"

namespace parser {

class Parser {
public:
	virtual ~Parser() = default;

	// Drives the builder. True if the input was accepted. The product is taken
	// from the concrete builder after a successful parse.
	virtual bool parse(scanner::Scanner& scanner, SyntaxTreeBuilder& syntaxTreeBuilder) = 0;
};

} // namespace parser

#endif // PARSER_H_
