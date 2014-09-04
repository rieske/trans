#include "AbstractSyntaxTreeBuilder.h"

#include "AbstractSyntaxTree.h"
#include "AbstractSyntaxTreeNode.h"
#include "TerminalSymbol.h"

namespace ast {

AbstractSyntaxTreeBuilder::AbstractSyntaxTreeBuilder() {
}

AbstractSyntaxTreeBuilder::~AbstractSyntaxTreeBuilder() {
}

void AbstractSyntaxTreeBuilder::makeNonterminalNode(std::string definingSymbol, parser::Production production) {
	syntaxNodeBuilder.updateContext(definingSymbol, production.producedSequence(), treeBuilderContext);
}

void AbstractSyntaxTreeBuilder::makeTerminalNode(std::string type, std::string value, const TranslationUnitContext& context) {
	treeBuilderContext.pushTerminal( { type, value, context });
}

std::unique_ptr<parser::SyntaxTree> AbstractSyntaxTreeBuilder::build() {
	return std::unique_ptr<parser::SyntaxTree>(new AbstractSyntaxTree(treeBuilderContext.popStatement()));
}

}
