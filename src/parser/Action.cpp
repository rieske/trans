#include "Action.h"

#include <sstream>
#include <stdexcept>

#include "Grammar.h"
#include "ParsingTable.h"
#include "Production.h"
#include "util/Logger.h"
#include "util/LogManager.h"

namespace parser {

namespace {
const char SHIFT_ACTION = 's';
const char REDUCE_ACTION = 'r';
const char ERROR_ACTION = 'e';
const char ACCEPT_ACTION = 'a';

Logger& err = LogManager::getErrorLogger();
} // namespace

Action Action::shift(parse_state state) {
    Action action;
    action.kind_ = Kind::Shift;
    action.state_ = state;
    return action;
}

Action Action::reduce(const Production& production, const ParsingTable* parsingTable) {
    Action action;
    action.kind_ = Kind::Reduce;
    action.production_ = &production;
    action.parsingTable_ = parsingTable;
    return action;
}

Action Action::accept() {
    Action action;
    action.kind_ = Kind::Accept;
    return action;
}

Action Action::error(parse_state state,
        std::shared_ptr<const std::vector<int>> candidateSymbols,
        const Grammar* grammar) {
    Action action;
    action.kind_ = Kind::Error;
    action.state_ = state;
    action.candidateSymbols_ = std::move(candidateSymbols);
    action.grammar_ = grammar;
    return action;
}

std::string Action::serialize() const {
    switch (kind_) {
    case Kind::Accept:
        return std::string{ACCEPT_ACTION};
    case Kind::Shift:
        return std::string{SHIFT_ACTION} + " " + std::to_string(state_);
    case Kind::Reduce:
        return std::string{REDUCE_ACTION} + " " + std::to_string(production_->getId());
    case Kind::Error: {
        std::ostringstream s;
        s << ERROR_ACTION << " " << state_;
        if (candidateSymbols_) {
            for (const auto candidate : *candidateSymbols_) {
                s << " " << candidate;
            }
        }
        return s.str();
    }
    }
    __builtin_unreachable();
}

bool Action::equals(const Action& other) const {
    if (kind_ != other.kind_) {
        return false;
    }
    switch (kind_) {
    case Kind::Accept:
        return true;
    case Kind::Shift:
        return state_ == other.state_;
    case Kind::Reduce:
        return production_->getId() == other.production_->getId();
    case Kind::Error:
        return state_ == other.state_
                && candidateSymbols_ && other.candidateSymbols_
                && *candidateSymbols_ == *other.candidateSymbols_;
    }
    __builtin_unreachable();
}

Action Action::deserialize(const std::string& serializedAction,
        const ParsingTable& parsingTable, const Grammar& grammar) {
    std::istringstream actionStream { serializedAction };
    char type;
    actionStream >> type;
    switch (type) {
    case SHIFT_ACTION: {
        parse_state state;
        actionStream >> state;
        return Action::shift(state);
    }
    case REDUCE_ACTION: {
        size_t productionId;
        actionStream >> productionId;
        return Action::reduce(grammar.getRuleById(static_cast<int>(productionId)), &parsingTable);
    }
    case ERROR_ACTION: {
        parse_state state;
        actionStream >> state;
        std::vector<int> candidates;
        int candidate;
        while (actionStream >> candidate) {
            candidates.push_back(candidate);
        }
        return Action::error(state,
                std::make_shared<const std::vector<int>>(std::move(candidates)),
                &grammar);
    }
    case ACCEPT_ACTION:
        return Action::accept();
    default:
        throw std::runtime_error(
                "Error reading serialized parsing table: invalid action type: "
                        + std::to_string(type));
    }
}

bool Action::parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream,
        SyntaxTreeBuilder& syntaxTreeBuilder) const {
    switch (kind_) {
    case Kind::Accept:
        return true;
    case Kind::Shift: {
        parsingStack.push(state_);
        scanner::Token token = tokenStream.getCurrentToken();
        syntaxTreeBuilder.makeTerminalNode(token.id, token.lexeme, token.context);
        tokenStream.nextToken();
        return false;
    }
    case Kind::Reduce: {
        for (size_t i = production_->size(); i > 0; --i) {
            parsingStack.pop();
        }
        parsingStack.push(parsingTable_->go_to(parsingStack.top(), production_->getDefiningSymbol()));
        syntaxTreeBuilder.makeNonterminalNode(*production_);
        return false;
    }
    case Kind::Error: {
        syntaxTreeBuilder.err();
        scanner::Token currentToken = tokenStream.getCurrentToken();
        err << "Error: " << currentToken.context << ": unexpected token: " << currentToken.lexeme
                << " expected:";
        if (candidateSymbols_ && grammar_) {
            for (const auto candidate : *candidateSymbols_) {
                err << " " << grammar_->getSymbolById(candidate);
            }
        }
        err << "\n";
        return true;
    }
    }
    __builtin_unreachable();
}

} // namespace parser
