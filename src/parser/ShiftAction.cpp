#include "ShiftAction.h"

#include "../scanner/Token.h"
#include "../util/Logger.h"
#include "../util/LogManager.h"

using std::stack;
using std::string;

namespace parser {

ShiftAction::ShiftAction(parse_state state) :
		state { state } {
}

ShiftAction::~ShiftAction() {
}

bool ShiftAction::parse(stack<parse_state>& parsingStack, TokenStream& tokenStream, SemanticAnalyzer& semanticAnalyzer) const {
	parsingStack.push(state);
	Token token = tokenStream.getCurrentToken();
	semanticAnalyzer.makeTerminalNode(token.id, token.lexeme, token.line);
	tokenStream.nextToken();

	return false;
}

string ShiftAction::serialize() const {
	return "s " + std::to_string(state);
}

}
