#include "LR1Parser.h"

#include <stack>

#include "scanner/Token.h"
#include "LookaheadActionTable.h"
#include "ParsingTable.h"
#include "SyntaxTreeBuilder.h"
#include "TokenStream.h"
#include "Action.h"

using std::unique_ptr;

namespace parser {

LR1Parser::LR1Parser(ParsingTable* parsingTable) :
		parsingTable { parsingTable } {
}

LR1Parser::~LR1Parser() {
}

std::unique_ptr<SyntaxTree> LR1Parser::parse(Scanner& scanner, std::unique_ptr<SyntaxTreeBuilder> syntaxTreeBuilder) {
	TokenStream tokenStream { &scanner };

	std::stack<parse_state> parsingStack;
	parsingStack.push(0);
	while (!parsingTable->action(parsingStack.top(), tokenStream.getCurrentToken()).parse(parsingStack, tokenStream, syntaxTreeBuilder))
		;

	return syntaxTreeBuilder->build();
}

}
