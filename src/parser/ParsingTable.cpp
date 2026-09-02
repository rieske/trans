#include "ParsingTable.h"

#include "SyntaxTreeBuilder.h"
#include "util/Diagnostic.h"
#include "util/Logger.h"
#include "util/LogManager.h"

#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {
static Logger& logger = LogManager::getComponentLogger(Component::PARSER);

int terminalColumn(int symbolId, int minTerminal) {
    return symbolId - minTerminal;
}

int nonterminalColumn(int symbolId, int minNonterminal) {
    return symbolId - minNonterminal;
}

bool inTerminalRange(int symbolId, int minTerminal, int maxTerminal) {
    return symbolId >= minTerminal && symbolId <= maxTerminal;
}

bool inNonterminalRange(int symbolId, int minNonterminal, int maxNonterminal) {
    return symbolId >= minNonterminal && symbolId <= maxNonterminal;
}

std::size_t cellIndex(std::size_t state, int column, int columns) {
    return state * static_cast<std::size_t>(columns) + static_cast<std::size_t>(column);
}

} // namespace

namespace parser {

void ParsingTable::validate() const {
    logger << *grammar_;
    if (grammar_->ruleCount() != ruleCount_) {
        throw std::runtime_error { "compiled parsing table does not match grammar rule count" };
    }
    const std::vector<int>& terminals = grammar_->getTerminalIDs();
    if (terminals.size() != terminalIds_.size()) {
        throw std::runtime_error { "compiled parsing table does not match grammar terminals" };
    }
    for (std::size_t i = 0; i < terminals.size(); ++i) {
        if (terminals[i] != terminalIds_[i]) {
            throw std::runtime_error { "compiled parsing table does not match grammar terminals" };
        }
    }
}

ParsingTable::ActionCell ParsingTable::cell(parse_state state, int symbolId) const {
    if (symbolId < 0) {
        throw std::logic_error { "ParsingTable::cell: lookahead is not a grammar terminal" };
    }
    if (state < stateCount_ && inTerminalRange(symbolId, minTerminal_, maxTerminal_)
            && terminalColumns_ > 0) {
        const std::size_t index = cellIndex(state, terminalColumn(symbolId, minTerminal_), terminalColumns_);
        const uint8_t kind = actionKind_[index];
        if (kind == kCellShift || kind == kCellReduce || kind == kCellAccept) {
            return { kind, actionPayload_[index] };
        }
        if (kind != kCellEmpty) {
            throw std::logic_error { "ParsingTable::cell: invalid compiled cell kind" };
        }
    }
    return {};
}

Action ParsingTable::action(parse_state state, const scanner::Token& lookahead) const {
    static const auto kEmptyCandidates = std::make_shared<const std::vector<int>>();

    const ActionCell looked = cell(state, lookahead.symbolId);
    switch (looked.kind) {
    case kCellShift:
        return Action::shift(looked.payload);
    case kCellReduce:
        return Action::reduce(grammar_->getRuleById(looked.payload));
    case kCellAccept:
        return Action::accept();
    case kCellEmpty:
        break;
    default:
        throw std::logic_error { "ParsingTable::action: invalid compiled cell kind" };
    }
    if (state < stateCount_ && state + 1 < errorOffset_.size()) {
        const uint32_t begin = errorOffset_[state];
        const uint32_t end = errorOffset_[state + 1];
        if (begin != end) {
            return Action::error(0,
                    std::make_shared<const std::vector<int>>(
                            errorCandidates_.begin() + begin,
                            errorCandidates_.begin() + end));
        }
    }
    return Action::error(0, kEmptyCandidates);
}

void ParsingTable::reportError(parse_state state, const scanner::Token& current,
        SyntaxTreeBuilder& syntaxTreeBuilder) const {
    syntaxTreeBuilder.err();
    std::ostringstream message;
    message << "unexpected token: " << current.lexeme << " expected:";
    if (state < stateCount_ && state + 1 < errorOffset_.size()) {
        const uint32_t begin = errorOffset_[state];
        const uint32_t end = errorOffset_[state + 1];
        for (uint32_t i = begin; i < end; ++i) {
            message << " " << grammar_->getSymbolById(errorCandidates_[i]);
        }
    }
    syntaxTreeBuilder.sink().error(current.context, message.str());
}

std::optional<parse_state> ParsingTable::tryGoTo(parse_state state, int nonterminal) const {
    if (state >= stateCount_ || !inNonterminalRange(nonterminal, minNonterminal_, maxNonterminal_)
            || nonterminalColumns_ <= 0) {
        return std::nullopt;
    }
    const int16_t next = gotos_[cellIndex(state,
            nonterminalColumn(nonterminal, minNonterminal_), nonterminalColumns_)];
    if (next < 0) {
        return std::nullopt;
    }
    return static_cast<parse_state>(next);
}

parse_state ParsingTable::go_to(parse_state state, int nonterminal) const {
    const auto next = tryGoTo(state, nonterminal);
    if (!next) {
        throw std::out_of_range { "ParsingTable::go_to: missing edge" };
    }
    return *next;
}

} // namespace parser
