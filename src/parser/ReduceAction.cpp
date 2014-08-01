#include "ReduceAction.h"

#include "LR1Item.h"
#include "ParsingTable.h"
#include "Production.h"
#include "SyntaxTreeBuilder.h"

using std::stack;
using std::string;

namespace parser {

ReduceAction::ReduceAction(LR1Item reduction, const ParsingTable* parsingTable) :
		reduction { new LR1Item { reduction } },
		parsingTable { parsingTable } {
}

ReduceAction::~ReduceAction() {
}

bool ReduceAction::parse(stack<parse_state>& parsingStack, TokenStream&, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const {
	for (size_t i = reduction->getProduction().size(); i > 0; --i) {
		parsingStack.pop();
	}
	parsingStack.push(parsingTable->go_to(parsingStack.top(), reduction->getDefiningSymbol()));
	syntaxTreeBuilder->makeNonterminalNode(reduction->getDefiningSymbol(), reduction->getProduction());
	return false;
}

string ReduceAction::serialize() const {
	return "r " + reduction->getDefiningSymbol() + " " + std::to_string(reduction->getProductionNumber());
}

}
