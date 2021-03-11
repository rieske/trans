#include "ReduceAction.h"

namespace parser {

ReduceAction::ReduceAction(const Production& production, const ParsingTable* parsingTable) :
        production { production },
        parsingTable { parsingTable }
{
}

ReduceAction::~ReduceAction() {
}

bool ReduceAction::parse(std::stack<parse_state>& parsingStack, TokenStream&, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const {
    for (size_t i = production.size(); i > 0; --i) {
        parsingStack.pop();
    }
    parsingStack.push(parsingTable->go_to(parsingStack.top(), production.getDefiningSymbol().getId()));
    syntaxTreeBuilder->makeNonterminalNode(production.getDefiningSymbol().getId(), production);
    return false;
}

std::string ReduceAction::serialize() const {
    return "r " + std::to_string(production.getId());
}

} // namespace parser

