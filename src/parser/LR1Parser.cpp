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

LR1Parser::LR1Parser(std::unique_ptr<ParsingTable> parsingTable) :
    parsingTable { std::move(parsingTable) } {
}

LR1Parser::~LR1Parser() = default;

LrFinish runLrParse(const ParsingTable& parsingTable, TokenStream& tokenStream,
        SyntaxTreeBuilder& syntaxTreeBuilder, ParseExtensions* extensions,
        std::optional<LrStop> stop) {
    std::stack<parse_state> parsingStack;
    parsingStack.push(0);
    int nest = 0;
    for (;;) {
        const Action action = parsingTable.action(parsingStack.top(), tokenStream.getCurrentToken());
        // Prefix tokens for a dummy subparse are not live. Peeking them would
        // scan the first live token and corrupt nest.
        const bool live = !stop || stop->live == nullptr || *stop->live;
        if (extensions && live && action.kind() == Action::Kind::Shift) {
            if (const auto nextState = extensions->tryGoto(parsingStack.top(), tokenStream, parsingTable)) {
                if (extensions->accept(tokenStream, parsingTable, syntaxTreeBuilder)) {
                    parsingStack.push(*nextState);
                    continue;
                }
                if (syntaxTreeBuilder.hasError()) {
                    return LrFinish::Complete;
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
    }, extensions != nullptr };
    TokenStream tokenStream { [&filter]() {
        return filter.nextToken();
    }, scanner.session() };

    runLrParse(*parsingTable, tokenStream, syntaxTreeBuilder, extensions);
    return syntaxTreeBuilder.build();
}

} // namespace parser

