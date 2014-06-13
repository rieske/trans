#include "ErrorAction.h"

#include <stdexcept>

#include "../scanner/Token.h"
#include "../semantic_analyzer/SyntaxTree.h"
#include "GrammarSymbol.h"

using std::string;
using std::stack;
using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::ostringstream;

ErrorAction::ErrorAction(parse_state state, string forgeToken, shared_ptr<const GrammarSymbol> expectedSymbol, Logger logger) :
		Action { logger },
		state { state },
		forgeToken { forgeToken },
		expectedSymbol { expectedSymbol } {
}

ErrorAction::~ErrorAction() {
}

unique_ptr<SyntaxTree> ErrorAction::perform(stack<parse_state>& parsingStack, TokenStream& tokenStream,
		SemanticAnalyzer& semanticAnalyzer) {
	semanticAnalyzer.syntaxError();
	Token currentToken = tokenStream.getCurrentToken();

	if (currentToken.lexeme.empty()) {
		throw std::runtime_error("Error at end of input file! ");
	}
	std::cerr << "Error on line " << currentToken.line << ": " << *expectedSymbol << " expected, got: " << currentToken.lexeme << "\n";

	if (!forgeToken.empty() && !tokenStream.currentTokenIsForged()) {
		tokenStream.forgeToken( { forgeToken, forgeToken, currentToken.line });
		logger << "Inserting " << *expectedSymbol << " into input stream.\n";
	} else {
		parsingStack.push(state);
		tokenStream.nextToken();
		logger << "Stack: " << parsingStack.top() << "\tpush " << state << "\t\tlookahead: " << tokenStream.getCurrentToken().lexeme
				<< "\n";
	}
	return nullptr;
}

string ErrorAction::describe() const {
	ostringstream oss;
	oss << "e " << state << " " << (forgeToken.empty() ? "NOFORGE" : forgeToken) << " " << expectedSymbol->getName();
	return oss.str();
}
