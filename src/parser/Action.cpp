#include "Action.h"

#include <sstream>

#include "Grammar.h"
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

ErrorAction::ErrorAction(parse_state state, std::vector<int> candidateSymbols, const Grammar* grammar) :
    state { state },
    candidateSymbols { candidateSymbols },
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
    std::stringstream s;
    s << ERROR_ACTION << " " << state;
    for (const auto& candidate: candidateSymbols) {
        s << " " << candidate;
    }
    return s.str();
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
            actionStream >> state;
            std::vector<int> candidateSymbols;
            int candidate;
            while (actionStream >> candidate) {
                candidateSymbols.push_back(candidate);
            }
            return std::make_unique<ErrorAction>(state, candidateSymbols, &grammar);
        }
        case ACCEPT_ACTION:
            return std::make_unique<AcceptAction>();
        default:
            throw std::runtime_error("Error reading serialized parsing table: invalid action type: " + std::to_string(type));
    }
}

bool AcceptAction::parse(std::stack<parse_state>&, TokenStream&, SyntaxTreeBuilder&) const {
    return true;
}

bool ShiftAction::parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, SyntaxTreeBuilder& syntaxTreeBuilder) const {
    parsingStack.push(state);
    scanner::Token token = tokenStream.getCurrentToken();
    syntaxTreeBuilder.makeTerminalNode(token.id, token.lexeme, token.context);
    tokenStream.nextToken();
    return false;
}

bool ReduceAction::parse(std::stack<parse_state>& parsingStack, TokenStream&, SyntaxTreeBuilder& syntaxTreeBuilder) const {
    for (size_t i = production.size(); i > 0; --i) {
        parsingStack.pop();
    }
    parsingStack.push(parsingTable->go_to(parsingStack.top(), production.getDefiningSymbol()));
    syntaxTreeBuilder.makeNonterminalNode(production);
    return false;
}

bool ErrorAction::parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, SyntaxTreeBuilder& syntaxTreeBuilder) const {
    syntaxTreeBuilder.err();
    scanner::Token currentToken = tokenStream.getCurrentToken();

    err << "Error: " << currentToken.context << ": unexpected token: " << currentToken.lexeme << " expected:";
    for (const auto& candidate : candidateSymbols) {
        err << " " << grammar->getSymbolById(candidate);
    }
    err << "\n";
    return true;
}

bool Action::isCorrective() const {
    return false;
}

bool ReduceAction::isCorrective() const {
    return true;
}

} // namespace parser

