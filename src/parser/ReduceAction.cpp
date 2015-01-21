#include "ReduceAction.h"

#include <vector>

#include "GrammarSymbol.h"
#include "ParsingTable.h"
#include "Production.h"
#include "SyntaxTreeBuilder.h"

using std::stack;
using std::string;

namespace parser {

ReduceAction::ReduceAction(const GrammarSymbol* grammarSymbol, int productionNumber, const ParsingTable* parsingTable) :
        grammarSymbol { grammarSymbol },
        productionNumber { productionNumber },
        parsingTable { parsingTable }
{
}

ReduceAction::~ReduceAction() {
}

bool ReduceAction::parse(stack<parse_state>& parsingStack, TokenStream&, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const {
    Production production = grammarSymbol->getProductions().at(productionNumber);
    for (size_t i = production.size(); i > 0; --i) {
        parsingStack.pop();
    }
    parsingStack.push(parsingTable->go_to(parsingStack.top(), grammarSymbol->getDefinition()));
    syntaxTreeBuilder->makeNonterminalNode(grammarSymbol->getDefinition(), production);
    return false;
}

string ReduceAction::serialize() const {
    return "r " + grammarSymbol->getDefinition() + " " + std::to_string(productionNumber);
}

}
