#include "LR1Parser.h"

#include <fstream>
#include <iostream>
#include <stack>
#include <string>

#include "../scanner/Token.h"
#include "Action.h"
#include "LookaheadActionTable.h"
#include "ParsingTable.h"
#include "SyntaxTree.h"
#include "TokenStream.h"

using std::unique_ptr;

LR1Parser::LR1Parser(ParsingTable* parsingTable) :
		parsingTable { parsingTable } {
}

LR1Parser::~LR1Parser() {
}

void LR1Parser::parse(Scanner& scanner, SemanticAnalyzer& semanticAnalyzer) {
	TokenStream tokenStream { &scanner };
	tokenStream.nextToken();

	std::stack<parse_state> parsingStack;
	parsingStack.push(0);
	while (!parsingTable->action(parsingStack.top(), tokenStream.getCurrentToken()).parse(parsingStack, tokenStream, semanticAnalyzer))
		;
}
