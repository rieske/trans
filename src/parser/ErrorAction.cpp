#include "ErrorAction.h"

#include <iostream>
#include <memory>
#include <stdexcept>

#include "../scanner/Token.h"
#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "ErrorSyntaxTreeBuilder.h"

using std::string;
using std::stack;
using std::string;

namespace parser {

static Logger& logger = LogManager::getComponentLogger(Component::PARSER);

ErrorAction::ErrorAction(parse_state state, string forgeToken, string expectedSymbol) :
		state { state },
		forgeToken { forgeToken },
		expectedSymbol { expectedSymbol } {
}

ErrorAction::~ErrorAction() {
}

bool ErrorAction::parse(stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const {
	syntaxTreeBuilder.reset(new ErrorSyntaxTreeBuilder());
	Token currentToken = tokenStream.getCurrentToken();

	if (currentToken.lexeme.empty()) {
		throw std::runtime_error("Error at end of input file! ");
	}
	std::cerr << "Error: " << currentToken.context << ": " << expectedSymbol << " expected, got: " << currentToken.lexeme << "\n";

	if ((!forgeToken.empty() && forgeToken != "NOFORGE") && !tokenStream.currentTokenIsForged()) {
		tokenStream.forgeToken( { forgeToken, forgeToken, currentToken.context });
		logger << "Inserting " << expectedSymbol << " into input stream.\n";
	} else {
		parsingStack.push(state);
		tokenStream.nextToken();
	}
	return false;
}

string ErrorAction::serialize() const {
	return "e " + std::to_string(state) + " " + (forgeToken.empty() ? "NOFORGE" : forgeToken) + " " + expectedSymbol;
}

}
