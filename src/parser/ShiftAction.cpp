#include "ShiftAction.h"

#include <memory>

#include "../scanner/Token.h"
#include "SyntaxTreeBuilder.h"

using std::stack;
using std::string;

namespace parser {

ShiftAction::ShiftAction(parse_state state) :
		state { state } {
}

ShiftAction::~ShiftAction() {
}

bool ShiftAction::parse(stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const {
	parsingStack.push(state);
	Token token = tokenStream.getCurrentToken();
	syntaxTreeBuilder->makeTerminalNode(token.id, token.lexeme, token.context);
	tokenStream.nextToken();

	return false;
}

string ShiftAction::serialize() const {
	return "s " + std::to_string(state);
}

}
