#ifndef PARSER_H_
#define PARSER_H_

#include <memory>

#include "SyntaxTree.h"
#include "scanner/Scanner.h"

namespace parser {

class SyntaxTreeBuilder;

class Parser {
public:
	virtual ~Parser() = default;

	virtual std::unique_ptr<SyntaxTree> parse(Scanner& scanner, std::unique_ptr<SyntaxTreeBuilder> syntaxTreeBuilder) = 0;
};

}

#endif /* PARSER_H_ */
