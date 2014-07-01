#include "ReduceAction.h"

#include <vector>

#include "../scanner/Token.h"
#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "LR1Item.h"
#include "SyntaxTree.h"

using std::stack;
using std::unique_ptr;
using std::string;
using std::ostringstream;

static Logger& logger = LogManager::getComponentLogger(Component::PARSER);

ReduceAction::ReduceAction(LR1Item reduction, const ParsingTable* parsingTable) :
		reduction { new LR1Item { reduction } },
		parsingTable { parsingTable } {
}

ReduceAction::~ReduceAction() {
}

unique_ptr<SyntaxTree> ReduceAction::perform(stack<parse_state>& parsingStack, TokenStream& tokenStream,
		SemanticAnalyzer& semanticAnalyzer) const {

	logger << *reduction;

	for (size_t i = reduction->getProduction().size(); i > 0; i--) {
		logger << "Stack: " << parsingStack.top() << "\tpop " << parsingStack.top() << "\n";
		parsingStack.pop();
	}
	parse_state state = parsingTable->go_to(parsingStack.top(), reduction->getDefiningSymbol());

	logger << "Stack: " << parsingStack.top() << "\tpush " << state << "\t\tlookahead: " << tokenStream.getCurrentToken().lexeme << "\n";

	parsingStack.push(state);
	semanticAnalyzer.makeNonterminalNode(reduction->getDefiningSymbol(), reduction->getProduction());

	return nullptr;
}

string ReduceAction::serialize() const {
	return "r " + reduction->getDefiningSymbol() + " " + std::to_string(reduction->getProductionNumber());
}
