#ifndef PARSER_H_
#define PARSER_H_

#include <memory>

#include "SyntaxTree.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/Scanner.h"

namespace parser {

class Parser {
public:
	virtual ~Parser() = default;

	virtual std::unique_ptr<SyntaxTree> parse(scanner::Scanner& scanner, SyntaxTreeBuilder& syntaxTreeBuilder) = 0;
};

} // namespace parser

#endif // PARSER_H_
