#include "ReduceAction.h"

#include "GrammarSymbol.h"
#include "ParsingTable.h"
#include "SyntaxTreeBuilder.h"

using std::stack;
using std::string;

namespace parser {

ReduceAction::ReduceAction(const Production& production, const ParsingTable* parsingTable) :
        production { production },
        parsingTable { parsingTable }
{
}

ReduceAction::~ReduceAction() {
}

bool ReduceAction::parse(stack<parse_state>& parsingStack, TokenStream&, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const {
    for (size_t i = production.size(); i > 0; --i) {
        parsingStack.pop();
    }
    parsingStack.push(parsingTable->go_to(parsingStack.top(), production.getDefiningSymbol().getDefinition()));
    syntaxTreeBuilder->makeNonterminalNode(production.getDefiningSymbol().getDefinition(), production);
    return false;
}

string ReduceAction::serialize() const {
    return "r " + std::to_string(production.getId());
}

}
