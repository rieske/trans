#include "AbstractSyntaxTreeBuilder.h"

#include "AbstractSyntaxTree.h"
#include "AbstractSyntaxTreeNode.h"
#include "TerminalSymbol.h"

namespace ast {

AbstractSyntaxTreeBuilder::~AbstractSyntaxTreeBuilder() = default;

void AbstractSyntaxTreeBuilder::makeNonterminalNode(std::string definingSymbol, parser::Production production) {
	syntaxNodeBuilder.updateContext(definingSymbol, production.producedSequence(), treeBuilderContext);
}

void AbstractSyntaxTreeBuilder::makeTerminalNode(std::string type, std::string value, const translation_unit::Context& context) {
	treeBuilderContext.pushTerminal( { type, value, context });
}

std::unique_ptr<parser::SyntaxTree> AbstractSyntaxTreeBuilder::build() {
	return std::make_unique<AbstractSyntaxTree>(treeBuilderContext.popTranslationUnit());
}

} // namespace ast
