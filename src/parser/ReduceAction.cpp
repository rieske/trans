#include "ReduceAction.h"

#include "../semantic_analyzer/SyntaxTree.h"

using std::stack;
using std::unique_ptr;
using std::string;
using std::ostringstream;

ReduceAction::ReduceAction(LR1Item reduction,
		const std::map<parse_state, std::map<std::shared_ptr<const GrammarSymbol>, parse_state>>* gotoTable, Logger logger) :
		Action { logger },
		reduction { new LR1Item { reduction } },
		gotoTable { gotoTable } {
}

ReduceAction::~ReduceAction() {
}

unique_ptr<SyntaxTree> ReduceAction::perform(stack<parse_state>& parsingStack, TokenStream& tokenStream,
		SemanticAnalyzer& semanticAnalyzer) {

	logger << *reduction;

	for (size_t i = reduction->getProduction().size(); i > 0; i--) {
		logger << "Stack: " << parsingStack.top() << "\tpop " << parsingStack.top() << "\n";
		parsingStack.pop();
	}
	parse_state state = gotoTable->at(parsingStack.top()).at(reduction->getDefiningSymbol());

	logger << "Stack: " << parsingStack.top() << "\tpush " << state << "\t\tlookahead: " << tokenStream.getCurrentToken().lexeme << "\n";

	parsingStack.push(state);
	semanticAnalyzer.makeNonTerminalNode(reduction->getDefiningSymbol()->getName(), reduction->getProduction().size(),
			reduction->productionStr());

	return nullptr;
}

string ReduceAction::describe() const {
	ostringstream oss;
	oss << "r 0 " << reduction->getDefiningSymbol()->getId() << " " << reduction->getProductionId();
	return oss.str();
}
