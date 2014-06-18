#include "ShiftAction.h"

#include <memory>
#include <sstream>
#include <stack>
#include <string>

#include "../scanner/Token.h"
#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "../semantic_analyzer/SyntaxTree.h"
#include "../util/LogManager.h"
#include "TokenStream.h"

using std::stack;
using std::unique_ptr;
using std::string;
using std::ostringstream;

static Logger& logger = LogManager::getComponentLogger(Component::PARSER);

ShiftAction::ShiftAction(parse_state state) :
		state { state } {
}

ShiftAction::~ShiftAction() {
}

unique_ptr<SyntaxTree> ShiftAction::perform(stack<parse_state>& parsingStack, TokenStream& tokenStream,
		SemanticAnalyzer& semanticAnalyzer) const {

	logger << "Stack: " << parsingStack.top() << "\tpush " << state << "\t\tlookahead: " << tokenStream.getCurrentToken().lexeme << "\n";
	parsingStack.push(state);
	semanticAnalyzer.makeTerminalNode(tokenStream.getCurrentToken());
	tokenStream.nextToken();

	return nullptr;
}

string ShiftAction::serialize() const {
	ostringstream oss;
	oss << "s " << state;
	return oss.str();
}
