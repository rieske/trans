#include "LR1Parser.h"

#include <stack>
#include <string>

#include "ParseExtensions.h"
#include "ParsingTable.h"
#include "SyntaxTreeBuilder.h"
#include "TokenStream.h"
#include "Action.h"
#include "scanner/Token.h"
#include "scanner/TokenFilter.h"

namespace parser {

namespace {

enum class StopDecision {
    Continue,
    ReduceAndStop,
    PeekStop,
};

bool isAssignmentOperator(const std::string& id) {
    return id == "=" || id == "*=" || id == "/=" || id == "%="
            || id == "+=" || id == "-=" || id == "<<=" || id == ">>="
            || id == "&=" || id == "^=" || id == "|=";
}

// True when reducing `action` would leave a state that immediately reduces the
// same defining symbol again (e.g. outer cast_exp after its operand cast_exp).
bool sameSymbolGrows(const Action& action, const std::stack<parse_state>& parsingStack,
        const scanner::Token& lookahead, int symbol, const ParsingTable& parsingTable) {
    const auto rhsSize = action.reduceRhsSize();
    if (!rhsSize || parsingStack.size() <= *rhsSize) {
        return false;
    }
    std::stack<parse_state> after = parsingStack;
    for (std::size_t i = 0; i < *rhsSize; ++i) {
        after.pop();
    }
    const auto nextState = parsingTable.tryGoTo(after.top(), symbol);
    if (!nextState) {
        return false;
    }
    const Action next = parsingTable.action(*nextState, lookahead);
    return next.reduceDefiningSymbol() == symbol;
}

StopDecision stopDecision(const LrStop& stop, const Action& action,
        const std::stack<parse_state>& parsingStack, const scanner::Token& lookahead, int nest,
        const ParsingTable& parsingTable) {
    if (nest != 0) {
        return StopDecision::Continue;
    }
    if (stop.mode == LrStop::Mode::Lookahead) {
        if (action.reduceDefiningSymbol() == stop.definingSymbol
                && stop.lookahead && lookahead.id == *stop.lookahead) {
            return StopDecision::ReduceAndStop;
        }
        return StopDecision::Continue;
    }
    // Mode::Complete: nonterminal finished, including stop-before-assignment-shift.
    if (action.reduceDefiningSymbol() == stop.definingSymbol) {
        if (!sameSymbolGrows(action, parsingStack, lookahead, stop.definingSymbol, parsingTable)) {
            return StopDecision::ReduceAndStop;
        }
        return StopDecision::Continue;
    }
    // Assignment shifts at unary_exp and never reduces unary -> cast_exp.
    // PeekStop is part of Complete mode only (not Lookahead).
    if (action.kind() == Action::Kind::Shift && isAssignmentOperator(lookahead.id)) {
        return StopDecision::PeekStop;
    }
    return StopDecision::Continue;
}

} // namespace

LrStop LrStop::untilLookahead(int definingSymbol, std::string lookahead, const bool* live) {
    return LrStop { definingSymbol, Mode::Lookahead, live, std::move(lookahead) };
}

LrStop LrStop::untilComplete(int definingSymbol, const bool* live) {
    return LrStop { definingSymbol, Mode::Complete, live, std::nullopt };
}

LR1Parser::LR1Parser(const ParsingTable& parsingTable) :
    parsingTable { parsingTable } {
}

LR1Parser::~LR1Parser() = default;

namespace {

// Representative terminal from FIRST(<type_spec>) for this product grammar.
// Pure-reduce states list ISO type keywords in FOLLOW, not extension `id`
// spellings (__int128). Probing this terminal yields the same Reduce that
// any type-spec first token would; the real token stays current for retry.
constexpr const char* kTypeSpecFirstProbe = "int";

} // namespace

LrFinish runLrParse(const ParsingTable& parsingTable, TokenStream& tokenStream,
        SyntaxTreeBuilder& syntaxTreeBuilder, ParseExtensions* extensions,
        std::optional<LrStop> stop) {
    std::stack<parse_state> parsingStack;
    parsingStack.push(0);
    int nest = 0;
    for (;;) {
        const scanner::Token current = tokenStream.getCurrentToken();
        const Action action = parsingTable.action(parsingStack.top(), current);
        // Prefix tokens for a dummy subparse are not live. Peeking them would
        // scan the first live token and corrupt nest.
        const bool live = !stop || stop->live == nullptr || *stop->live;
        if (extensions && live) {
            if (const auto nextState = extensions->tryGoto(parsingStack.top(), tokenStream, parsingTable)) {
                if (extensions->accept(tokenStream, parsingTable, syntaxTreeBuilder)) {
                    parsingStack.push(*nextState);
                    continue;
                }
                if (syntaxTreeBuilder.hasError()) {
                    return LrFinish::Complete;
                }
            } else if (action.kind() == Action::Kind::Error
                    && extensions->isTypeExtensionToken(current)) {
                const scanner::Token typeProbe {
                        kTypeSpecFirstProbe, kTypeSpecFirstProbe, current.context };
                const Action asTypeKeyword = parsingTable.action(parsingStack.top(), typeProbe);
                if (asTypeKeyword.kind() == Action::Kind::Reduce) {
                    asTypeKeyword.parse(parsingStack, tokenStream, syntaxTreeBuilder);
                    continue;
                }
            }
        }
        if (stop && live) {
            const StopDecision decision = stopDecision(*stop, action, parsingStack, current, nest,
                    parsingTable);
            if (decision == StopDecision::ReduceAndStop) {
                action.parse(parsingStack, tokenStream, syntaxTreeBuilder);
                return LrFinish::Stopped;
            }
            if (decision == StopDecision::PeekStop) {
                return LrFinish::Stopped;
            }
        }
        if (stop && live && action.kind() == Action::Kind::Shift) {
            const std::string& id = current.id;
            const bool open = id == "(" || id == "[" || id == "{";
            const bool close = id == ")" || id == "]" || id == "}";
            if (open) {
                ++nest;
            } else if (close && nest > 0) {
                --nest;
            }
        }
        if (action.parse(parsingStack, tokenStream, syntaxTreeBuilder)) {
            return LrFinish::Complete;
        }
    }
}

std::unique_ptr<SyntaxTree> LR1Parser::parse(scanner::Scanner& scanner, SyntaxTreeBuilder& syntaxTreeBuilder) {
    ParseExtensions* extensions = syntaxTreeBuilder.parseExtensions();
    scanner::TokenFilter filter { [&scanner]() {
        return scanner.nextToken();
    }, extensions != nullptr };
    TokenStream tokenStream { [&filter]() {
        return filter.nextToken();
    }, scanner.session() };

    runLrParse(parsingTable, tokenStream, syntaxTreeBuilder, extensions);
    return syntaxTreeBuilder.build();
}

} // namespace parser
