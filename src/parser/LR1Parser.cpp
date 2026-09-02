#include "LR1Parser.h"

#include <string>
#include <vector>

#include "ParseExtensions.h"
#include "ParsingTable.h"
#include "Production.h"
#include "SyntaxTreeBuilder.h"
#include "TokenStream.h"
#include "Action.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"
#include "scanner/TokenFilter.h"
#include "util/Diagnostic.h"

namespace parser {

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

void applyShift(std::vector<parse_state>& stack, parse_state next, TokenStream& tokenStream,
        SyntaxTreeBuilder& syntaxTreeBuilder) {
    stack.push_back(next);
    const scanner::Token& token = tokenStream.getCurrentToken();
    syntaxTreeBuilder.makeTerminalNode(std::string { token.id }, std::string { token.lexeme }, token.context);
    tokenStream.nextToken();
}

bool applyReduce(std::vector<parse_state>& stack, const Production& production,
        const ParsingTable& parsingTable, SyntaxTreeBuilder& syntaxTreeBuilder) {
    stack.resize(stack.size() - production.size());
    stack.push_back(parsingTable.go_to(stack.back(), production.getDefiningSymbol()));
    syntaxTreeBuilder.makeNonterminalNode(production);
    return syntaxTreeBuilder.aborted();
}

} // namespace

LrFinish runLrParse(const ParsingTable& parsingTable, TokenStream& tokenStream,
        SyntaxTreeBuilder& syntaxTreeBuilder, ParseExtensions* extensions,
        std::optional<LrStop> stop) {
    std::vector<parse_state> parsingStack { 0 };
    int nest = 0;
    const Grammar* grammar = parsingTable.getGrammar();
    const int lparenId = grammar->trySymbolId("(").value_or(-2);
    const int lbracketId = grammar->trySymbolId("[").value_or(-2);
    const int rparenId = grammar->trySymbolId(")").value_or(-2);
    const int rbracketId = grammar->trySymbolId("]").value_or(-2);
    for (;;) {
        if (syntaxTreeBuilder.aborted()) {
            return LrFinish::Complete;
        }
        const scanner::Token& current = tokenStream.getCurrentToken();
        const parse_state state = parsingStack.back();
        const ParsingTable::ActionCell cell = parsingTable.cell(state, current.symbolId);
        const bool live = !stop || stop->live == nullptr || *stop->live;
        if (extensions && live) {
            if (const auto nextState = extensions->tryGoto(state, tokenStream, parsingTable)) {
                if (extensions->accept(tokenStream, parsingTable, syntaxTreeBuilder)) {
                    parsingStack.push_back(*nextState);
                    continue;
                }
                if (syntaxTreeBuilder.aborted()) {
                    return LrFinish::Complete;
                }
            } else if (cell.kind == ParsingTable::kCellEmpty
                    && extensions->isTypeExtensionToken(current)) {
                const auto probeId = grammar->trySymbolId(kTypeSpecFirstProbe);
                if (probeId) {
                    const ParsingTable::ActionCell asType = parsingTable.cell(state, *probeId);
                    if (asType.kind == ParsingTable::kCellReduce) {
                        if (applyReduce(parsingStack, grammar->getRuleById(asType.payload),
                                parsingTable, syntaxTreeBuilder)) {
                            return LrFinish::Complete;
                        }
                        continue;
                    }
                }
            }
        }
        if (stop
                && live
                && cell.kind == ParsingTable::kCellReduce
                && grammar->getRuleById(cell.payload).getDefiningSymbol() == stop->definingSymbol
                && tokenStream.getCurrentToken().id == stop->lookahead
                && nest == 0) {
            if (applyReduce(parsingStack, grammar->getRuleById(cell.payload), parsingTable, syntaxTreeBuilder)) {
                return LrFinish::Complete;
            }
            return LrFinish::Stopped;
        }
        if (stop && live && cell.kind == ParsingTable::kCellShift) {
            const int id = tokenStream.getCurrentToken().symbolId;
            if (id == lparenId || id == lbracketId) {
                ++nest;
            } else if ((id == rparenId || id == rbracketId) && nest > 0) {
                --nest;
            }
        }
        switch (cell.kind) {
        case ParsingTable::kCellAccept:
            return LrFinish::Complete;
        case ParsingTable::kCellShift:
            applyShift(parsingStack, cell.payload, tokenStream, syntaxTreeBuilder);
            break;
        case ParsingTable::kCellReduce:
            if (applyReduce(parsingStack, grammar->getRuleById(cell.payload), parsingTable, syntaxTreeBuilder)) {
                return LrFinish::Complete;
            }
            break;
        default:
            parsingTable.action(state, current).reportError(tokenStream, syntaxTreeBuilder);
            return LrFinish::Complete;
        }
    }
}

std::unique_ptr<SyntaxTree> LR1Parser::parse(scanner::Scanner& scanner, SyntaxTreeBuilder& syntaxTreeBuilder) {
    ParseExtensions* extensions = syntaxTreeBuilder.parseExtensions();
    scanner::TokenFilter filter { [&scanner]() {
        return scanner.nextToken();
    }, extensions != nullptr, &scanner.session() };
    TokenStream tokenStream { [&filter]() {
        return filter.nextToken();
    }, scanner.session(), *parsingTable.getGrammar() };

    try {
        runLrParse(parsingTable, tokenStream, syntaxTreeBuilder, extensions);
    } catch (const scanner::LexError& error) {
        if (syntaxTreeBuilder.hasSink()) {
            syntaxTreeBuilder.sink().error(error.where, error.what());
            return nullptr;
        }
        throw;
    }
    if (syntaxTreeBuilder.aborted()) {
        return nullptr;
    }
    return syntaxTreeBuilder.build();
}

} // namespace parser

