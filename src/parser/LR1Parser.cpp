#include "LR1Parser.h"

#include <stack>

#include "ParsingTable.h"
#include "SyntaxTreeBuilder.h"
#include "TokenStream.h"
#include "Action.h"

namespace parser {

LR1Parser::LR1Parser(ParsingTable* parsingTable) :
    parsingTable { parsingTable } {
}

LR1Parser::~LR1Parser() = default;

std::unique_ptr<SyntaxTree> LR1Parser::parse(scanner::Scanner& scanner, SyntaxTreeBuilder& syntaxTreeBuilder) {
    TokenStream tokenStream { [&scanner]() {
        return scanner.nextToken();
    }};

    std::stack<parse_state> parsingStack;
    parsingStack.push(0);
    while (!parsingTable->action(parsingStack.top(), tokenStream.getCurrentToken()).parse(parsingStack, tokenStream, syntaxTreeBuilder))
        ;

    return syntaxTreeBuilder.build();
}

} // namespace parser

