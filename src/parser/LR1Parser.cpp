#include "LR1Parser.h"

#include <stack>

#include "ParseExtensions.h"
#include "ParsingTable.h"
#include "SyntaxTreeBuilder.h"
#include "TokenStream.h"
#include "Action.h"
#include "scanner/Token.h"
#include "scanner/TokenFilter.h"

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
        if (stop
                && live
                && action.reduceDefiningSymbol() == stop->definingSymbol
                && tokenStream.getCurrentToken().id == stop->lookahead
                && nest == 0) {
            action.parse(parsingStack, tokenStream, syntaxTreeBuilder);
            return LrFinish::Stopped;
        }
        if (stop && live && action.kind() == Action::Kind::Shift) {
            const std::string& id = tokenStream.getCurrentToken().id;
            if (id == "(" || id == "[") {
                ++nest;
            } else if ((id == ")" || id == "]") && nest > 0) {
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
    }, extensions != nullptr, &scanner.session() };
    TokenStream tokenStream { [&filter]() {
        return filter.nextToken();
    }, scanner.session() };

    runLrParse(parsingTable, tokenStream, syntaxTreeBuilder, extensions);
    return syntaxTreeBuilder.build();
}

} // namespace parser

