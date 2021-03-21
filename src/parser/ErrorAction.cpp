#include "ErrorAction.h"

#include "scanner/Token.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "ErrorSyntaxTreeBuilder.h"

namespace {
static Logger& err = LogManager::getErrorLogger();
} // namespace

namespace parser {

ErrorAction::ErrorAction(parse_state state, int expectedSymbol, const Grammar* grammar) :
        state { state },
        expectedSymbol { expectedSymbol },
        grammar { grammar }
{
}

ErrorAction::~ErrorAction() {
}

bool ErrorAction::parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const {
    syntaxTreeBuilder.reset(new ErrorSyntaxTreeBuilder());
    scanner::Token currentToken = tokenStream.getCurrentToken();

    std::string expectedTerminal = grammar->getSymbolById(expectedSymbol);
    err << "Error: " << currentToken.context << ": " << expectedTerminal << " expected, got: " << currentToken.lexeme << "\n";
    return true;
}

std::string ErrorAction::serialize() const {
    return "e " + std::to_string(state) + " " + std::to_string(expectedSymbol);
}

} // namespace parser

