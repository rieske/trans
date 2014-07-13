#ifndef PARSER_H_
#define PARSER_H_

#include "../semantic_analyzer/SemanticAnalyzer.h"

class Scanner;

namespace parser {

class Parser {
public:
	virtual ~Parser() {
	}

	virtual void parse(Scanner& scanner, SemanticAnalyzer& semanticAnalyzer) = 0;
};

}

#endif /* PARSER_H_ */
