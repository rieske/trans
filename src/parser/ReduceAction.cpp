#include "ReduceAction.h"

#include <vector>

#include "../scanner/Token.h"
#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "LR1Item.h"
#include "ParsingTable.h"

using std::stack;
using std::string;

namespace parser {

ReduceAction::ReduceAction(LR1Item reduction, const ParsingTable* parsingTable) :
		reduction { new LR1Item { reduction } },
		parsingTable { parsingTable } {
}

ReduceAction::~ReduceAction() {
}

bool ReduceAction::parse(stack<parse_state>& parsingStack, TokenStream&, SemanticAnalyzer& semanticAnalyzer) const {
	for (size_t i = reduction->getProduction().size(); i > 0; --i) {
		parsingStack.pop();
	}
	parsingStack.push(parsingTable->go_to(parsingStack.top(), reduction->getDefiningSymbol()));
	semanticAnalyzer.makeNonterminalNode(reduction->getDefiningSymbol(), reduction->getProduction());
	return false;
}

string ReduceAction::serialize() const {
	return "r " + reduction->getDefiningSymbol() + " " + std::to_string(reduction->getProductionNumber());
}

}
