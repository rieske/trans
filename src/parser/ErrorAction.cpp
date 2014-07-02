#include "ErrorAction.h"

#include <iostream>
#include <stdexcept>

#include "../scanner/Token.h"
#include "../util/Logger.h"
#include "../util/LogManager.h"

using std::string;
using std::stack;
using std::string;

static Logger& logger = LogManager::getComponentLogger(Component::PARSER);

ErrorAction::ErrorAction(parse_state state, string forgeToken, string expectedSymbol) :
		state { state },
		forgeToken { forgeToken },
		expectedSymbol { expectedSymbol } {
}

ErrorAction::~ErrorAction() {
}

bool ErrorAction::parse(stack<parse_state>& parsingStack, TokenStream& tokenStream, SemanticAnalyzer& semanticAnalyzer) const {
	semanticAnalyzer.syntaxError();
	Token currentToken = tokenStream.getCurrentToken();

	if (currentToken.lexeme.empty()) {
		throw std::runtime_error("Error at end of input file! ");
	}
	std::cerr << "Error on line " << currentToken.line << ": " << expectedSymbol << " expected, got: " << currentToken.lexeme << "\n";

	if ((!forgeToken.empty() && forgeToken != "NOFORGE") && !tokenStream.currentTokenIsForged()) {
		tokenStream.forgeToken( { forgeToken, forgeToken, currentToken.line });
		logger << "Inserting " << expectedSymbol << " into input stream.\n";
	} else {
		parsingStack.push(state);
		tokenStream.nextToken();
		logger << "Stack: " << parsingStack.top() << "\tpush " << state << "\t\tlookahead: " << tokenStream.getCurrentToken().lexeme
				<< "\n";
	}
	return false;
}

string ErrorAction::serialize() const {
	return "e " + std::to_string(state) + " " + (forgeToken.empty() ? "NOFORGE" : forgeToken) + " " + expectedSymbol;
}
