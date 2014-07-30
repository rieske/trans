#include "SemanticTreeBuilder.h"

#include "AbstractSyntaxTree.h"
#include "AbstractSyntaxTreeNode.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

SemanticTreeBuilder::SemanticTreeBuilder() {
}

SemanticTreeBuilder::~SemanticTreeBuilder() {
}

void SemanticTreeBuilder::makeNonterminalNode(std::string definingSymbol, parser::Production production) {
	syntaxNodeFactory.updateContext(definingSymbol, production.producedSequence(), context);
}

void SemanticTreeBuilder::makeTerminalNode(std::string type, std::string value, size_t line) {
	context.setLine(line);
	context.pushTerminal( { type, value, line });
}

std::unique_ptr<parser::SyntaxTree> SemanticTreeBuilder::build() {
	return std::unique_ptr<parser::SyntaxTree>(new AbstractSyntaxTree(context.popStatement(), context.scope()));
}

}
