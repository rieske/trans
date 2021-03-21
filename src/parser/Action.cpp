#include "Action.h"

#include <sstream>

#include "Grammar.h"
#include "parser/ErrorSyntaxTreeBuilder.h"
#include "util/Logger.h"
#include "util/LogManager.h"

namespace parser {

const char SHIFT_ACTION = 's';
const char REDUCE_ACTION = 'r';
const char ERROR_ACTION = 'e';
const char ACCEPT_ACTION = 'a';

static Logger& err = LogManager::getErrorLogger();

ShiftAction::ShiftAction(parse_state state) :
    state { state }
{
}

ReduceAction::ReduceAction(const Production& production, const ParsingTable* parsingTable) :
    production { production },
    parsingTable { parsingTable }
{
}

ErrorAction::ErrorAction(parse_state state, int expectedSymbol, const Grammar* grammar) :
    state { state },
    expectedSymbol { expectedSymbol },
    grammar { grammar }
{
}

Action::~Action() = default;
AcceptAction::~AcceptAction() = default;
ShiftAction::~ShiftAction() = default;
ReduceAction::~ReduceAction() = default;
ErrorAction::~ErrorAction() = default;

std::string AcceptAction::serialize() const {
    return std::string{ACCEPT_ACTION};
}

std::string ShiftAction::serialize() const {
    return std::string{SHIFT_ACTION} + " " + std::to_string(state);
}

std::string ReduceAction::serialize() const {
    return std::string{REDUCE_ACTION} + " " + std::to_string(production.getId());
}

std::string ErrorAction::serialize() const {
    return std::string{ERROR_ACTION} + " " + std::to_string(state) + " " + std::to_string(expectedSymbol);
}

std::unique_ptr<Action> Action::deserialize(std::string serializedAction, const ParsingTable& parsingTable, const Grammar& grammar) {
    std::istringstream actionStream { serializedAction };
    char type;
    actionStream >> type;
    switch (type) {
        case SHIFT_ACTION: {
            parse_state state;
            actionStream >> state;
            return std::make_unique<ShiftAction>(state);
        }
        case REDUCE_ACTION: {
            size_t productionId;
            actionStream >> productionId;
            const auto& production = grammar.getRuleById(productionId);
            return std::make_unique<ReduceAction>(production, &parsingTable);
        }
        case ERROR_ACTION: {
            parse_state state;
            int expected;
            actionStream >> state >> expected;
            return std::make_unique<ErrorAction>(state, expected, &grammar);
        }
        case ACCEPT_ACTION:
            return std::make_unique<AcceptAction>();
        default:
            throw std::runtime_error("Error reading serialized parsing table: invalid action type: " + std::to_string(type));
    }
}


bool AcceptAction::parse(std::stack<parse_state>&, TokenStream&, std::unique_ptr<SyntaxTreeBuilder>&) const {
    return true;
}

bool ShiftAction::parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const {
    parsingStack.push(state);
    scanner::Token token = tokenStream.getCurrentToken();
    syntaxTreeBuilder->makeTerminalNode(token.id, token.lexeme, token.context);
    tokenStream.nextToken();
    return false;
}

bool ReduceAction::parse(std::stack<parse_state>& parsingStack, TokenStream&, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const {
    for (size_t i = production.size(); i > 0; --i) {
        parsingStack.pop();
    }
    parsingStack.push(parsingTable->go_to(parsingStack.top(), production.getDefiningSymbol()));
    syntaxTreeBuilder->makeNonterminalNode(production);
    return false;
}

bool ErrorAction::parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const {
    syntaxTreeBuilder.reset(new ErrorSyntaxTreeBuilder());
    scanner::Token currentToken = tokenStream.getCurrentToken();

    std::string expectedTerminal = grammar->getSymbolById(expectedSymbol);
    err << "Error: " << currentToken.context << ": " << expectedTerminal << " expected, got: " << currentToken.lexeme << "\n";
    return true;
}

} // namespace parser

