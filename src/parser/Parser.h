#ifndef PARSER_H_
#define PARSER_H_

#include <memory>

class Scanner;
class SemanticAnalyzer;
class SyntaxTree;

class Parser {
public:
	virtual ~Parser() {
	}

	virtual std::unique_ptr<SyntaxTree> parse(Scanner& scanner, SemanticAnalyzer& semanticAnalyzer) = 0;
};

#endif /* PARSER_H_ */
