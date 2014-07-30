#include "SemanticTreeBuilder.h"

#include <stdexcept>

#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTree.h"
#include "AbstractSyntaxTreeNode.h"
#include "TerminalSymbol.h"

using std::string;
using std::vector;

namespace semantic_analyzer {

SemanticTreeBuilder::SemanticTreeBuilder() :
		symbolTable { new SymbolTable() },
		context { symbolTable } {
}

SemanticTreeBuilder::~SemanticTreeBuilder() {
}

void SemanticTreeBuilder::makeNonterminalNode(string definingSymbol, parser::Production production) {
	syntaxNodeFactory.updateContext(definingSymbol, production.producedSequence(), context);
}

void SemanticTreeBuilder::makeTerminalNode(std::string type, std::string value, size_t line) {
	context.setLine(line);
	context.pushTerminal( { type, value, line });
}

std::unique_ptr<parser::SyntaxTree> SemanticTreeBuilder::build() {
	//containsSemanticErrors |= nonterminalNode->getErrorFlag(); FIXME: this used to be checked after
	// construction of each node. implement semantic checking in a separate visitor
	// probably better to throw on first semantic erro for now. TODO

	if (containsSemanticErrors) {
		throw std::runtime_error("Compilation failed with semantic errors!");
	}
	return std::unique_ptr<parser::SyntaxTree>(new AbstractSyntaxTree(context.popStatement(), symbolTable));
}

}
