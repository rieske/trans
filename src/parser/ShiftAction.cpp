#include "ShiftAction.h"

#include "../scanner/Token.h"
#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "SyntaxTree.h"

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
	return "s " + std::to_string(state);
}
