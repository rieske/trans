#include "LR1Parser.h"

#include <stack>

#include "ParsingTable.h"
#include "SyntaxTreeBuilder.h"
#include "TokenStream.h"
#include "Action.h"
#include "scanner/TokenFilter.h"

namespace parser {

LR1Parser::LR1Parser(std::unique_ptr<ParsingTable> parsingTable) :
    parsingTable { std::move(parsingTable) } {
}

LR1Parser::~LR1Parser() = default;

std::unique_ptr<SyntaxTree> LR1Parser::parse(scanner::Scanner& scanner, SyntaxTreeBuilder& syntaxTreeBuilder) {
    scanner::TokenFilter filter { [&scanner]() {
        return scanner.nextToken();
    } };
    TokenStream tokenStream { [&filter]() {
        return filter.nextToken();
    }, scanner.typedefs() };

    std::stack<parse_state> parsingStack;
    parsingStack.push(0);
    while (!parsingTable->action(parsingStack.top(), tokenStream.getCurrentToken()).parse(parsingStack, tokenStream, syntaxTreeBuilder))
        ;

    return syntaxTreeBuilder.build();
}

} // namespace parser

