#include "ErrorAction.h"

#include "scanner/Token.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "ErrorSyntaxTreeBuilder.h"

namespace {
static Logger& logger = LogManager::getComponentLogger(Component::PARSER);
static Logger& err = LogManager::getErrorLogger();
} // namespace

namespace parser {

ErrorAction::ErrorAction(parse_state state, std::string forgeToken, std::string expectedSymbol) :
        state { state },
        forgeToken { forgeToken },
        expectedSymbol { expectedSymbol }
{
}

ErrorAction::~ErrorAction() {
}

bool ErrorAction::parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const {
    syntaxTreeBuilder.reset(new ErrorSyntaxTreeBuilder());
    Token currentToken = tokenStream.getCurrentToken();

    if (currentToken.lexeme.empty()) {
        return true;
    }
    err << "Error: " << currentToken.context << ": " << expectedSymbol << " expected, got: " << currentToken.lexeme << "\n";

    if ((!forgeToken.empty() && forgeToken != "N") && !tokenStream.currentTokenIsForged()) {
        tokenStream.forgeToken( { forgeToken, forgeToken, currentToken.context });
        logger << "Inserting " << expectedSymbol << " into input stream.\n";
    } else {
        parsingStack.push(state);
        tokenStream.nextToken();
    }
    return false;
}

std::string ErrorAction::serialize() const {
    return "e " + std::to_string(state) + " " + (forgeToken.empty() ? "N" : forgeToken) + " " + expectedSymbol;
}

} // namespace parser

