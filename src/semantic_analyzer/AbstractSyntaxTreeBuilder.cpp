#include "AbstractSyntaxTreeBuilder.h"

#include "AbstractSyntaxTree.h"
#include "AbstractSyntaxTreeNode.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

AbstractSyntaxTreeBuilder::AbstractSyntaxTreeBuilder() {
}

AbstractSyntaxTreeBuilder::~AbstractSyntaxTreeBuilder() {
}

void AbstractSyntaxTreeBuilder::makeNonterminalNode(std::string definingSymbol, parser::Production production) {
	syntaxNodeBuilder.updateContext(definingSymbol, production.producedSequence(), context);
}

void AbstractSyntaxTreeBuilder::makeTerminalNode(std::string type, std::string value, size_t line) {
	context.setLine(line);
	context.pushTerminal( { type, value, line });
}

std::unique_ptr<parser::SyntaxTree> AbstractSyntaxTreeBuilder::build() {
	return std::unique_ptr<parser::SyntaxTree>(new AbstractSyntaxTree(context.popStatement()));
}

}
