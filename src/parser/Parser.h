#ifndef PARSER_H_
#define PARSER_H_

#include <memory>

class Scanner;
class TranslationUnit;
class SyntaxTree;

class Parser {
public:
	virtual ~Parser() {
	}

	virtual std::unique_ptr<SyntaxTree> parse(Scanner& scanner) = 0;
};

#endif /* PARSER_H_ */
