#include "ShiftAction.h"

namespace parser {

ShiftAction::ShiftAction(parse_state state) :
        state { state }
{
}

ShiftAction::~ShiftAction() {
}

bool ShiftAction::parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const {
    parsingStack.push(state);
    scanner::Token token = tokenStream.getCurrentToken();
    syntaxTreeBuilder->makeTerminalNode(token.id, token.lexeme, token.context);
    tokenStream.nextToken();
    return false;
}

std::string ShiftAction::serialize() const {
    return "s " + std::to_string(state);
}

} // namespace parser

