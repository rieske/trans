#include "ast/AbstractSyntaxTreeBuilder.h"

#include "ast/AbstractSyntaxTree.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/TerminalSymbol.h"

namespace ast {

void AbstractSyntaxTreeBuilder::makeNonterminalNode(std::string definingSymbol, parser::Production production) {
	syntaxNodeBuilder.updateContext(definingSymbol, production.producedSequence(), treeBuilderContext);
}

void AbstractSyntaxTreeBuilder::makeTerminalNode(std::string type, std::string value, const translation_unit::Context& context) {
	treeBuilderContext.pushTerminal( { type, value, context });
}

std::unique_ptr<parser::SyntaxTree> AbstractSyntaxTreeBuilder::build() {
	return std::make_unique<AbstractSyntaxTree>(treeBuilderContext.popTranslationUnit());
}

}
