#ifndef PARSER_H_
#define PARSER_H_

#include <memory>

class Scanner;

namespace parser {

class SyntaxTree;
class SyntaxTreeBuilder;

class Parser {
public:
	virtual ~Parser() {
	}

	virtual std::unique_ptr<SyntaxTree> parse(Scanner& scanner, std::unique_ptr<SyntaxTreeBuilder> syntaxTreeBuilder) = 0;
};

}

#endif /* PARSER_H_ */
