#include "Action.h"

#include <sstream>
#include <stdexcept>
#include <string>

#include "Grammar.h"
#include "ParsingTable.h"
#include "Production.h"
#include "util/Diagnostic.h"

namespace parser {

namespace {
const char SHIFT_ACTION = 's';
const char REDUCE_ACTION = 'r';
const char ERROR_ACTION = 'e';
const char ACCEPT_ACTION = 'a';
} // namespace

std::optional<int> Action::reduceDefiningSymbol() const {
    if (kind_ != Kind::Reduce || production_ == nullptr) {
        return std::nullopt;
    }
    return production_->getDefiningSymbol();
}

parse_state Action::shiftState() const {
    if (kind_ != Kind::Shift) {
        throw std::logic_error { "Action::shiftState: not a shift" };
    }
    return state_;
}

int Action::productionId() const {
    if (kind_ != Kind::Reduce || production_ == nullptr) {
        throw std::logic_error { "Action::productionId: not a reduce" };
    }
    return production_->getId();
}

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

std::string Action::toString() const {
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
    throw std::logic_error { "Action::toString: unhandled Kind" };
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
    throw std::logic_error { "Action::equals: unhandled Kind" };
}

bool Action::parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream,
        SyntaxTreeBuilder& syntaxTreeBuilder) const {
    switch (kind_) {
    case Kind::Accept:
        return true;
    case Kind::Shift: {
        parsingStack.push(state_);
        const scanner::Token& token = tokenStream.getCurrentToken();
        syntaxTreeBuilder.makeTerminalNode(std::string { token.id }, std::string { token.lexeme }, token.context);
        tokenStream.nextToken();
        return false;
    }
    case Kind::Reduce: {
        for (size_t i = production_->size(); i > 0; --i) {
            parsingStack.pop();
        }
        parsingStack.push(parsingTable_->go_to(parsingStack.top(), production_->getDefiningSymbol()));
        syntaxTreeBuilder.makeNonterminalNode(*production_);
        return syntaxTreeBuilder.aborted();
    }
    case Kind::Error:
        reportError(tokenStream, syntaxTreeBuilder);
        return true;
    }
    throw std::logic_error { "Action::parse: unhandled Kind" };
}

void Action::reportError(TokenStream& tokenStream, SyntaxTreeBuilder& syntaxTreeBuilder) const {
    syntaxTreeBuilder.err();
    const scanner::Token& currentToken = tokenStream.getCurrentToken();
    std::ostringstream message;
    message << "unexpected token: " << currentToken.lexeme << " expected:";
    if (candidateSymbols_ && grammar_) {
        for (const auto candidate : *candidateSymbols_) {
            message << " " << grammar_->getSymbolById(candidate);
        }
    }
    syntaxTreeBuilder.sink().error(currentToken.context, message.str());
}

} // namespace parser
