#include "AbstractSyntaxTreeBuilder.h"

#include "AbstractSyntaxTree.h"
#include "AbstractSyntaxTreeNode.h"
#include "TerminalSymbol.h"
#include "ast/ContextualSyntaxNodeBuilder.h"

namespace ast {

AbstractSyntaxTreeBuilder::AbstractSyntaxTreeBuilder(const parser::Grammar* grammar):
    syntaxNodeBuilder{*grammar}
{
}

AbstractSyntaxTreeBuilder::~AbstractSyntaxTreeBuilder() = default;

void AbstractSyntaxTreeBuilder::makeNonterminalNode(const parser::Production& production) {
	syntaxNodeBuilder.updateContext(production, treeBuilderContext);
}

void AbstractSyntaxTreeBuilder::makeTerminalNode(std::string type, std::string value, const translation_unit::Context& context) {
	treeBuilderContext.pushTerminal( { type, value, context });
}

std::unique_ptr<parser::SyntaxTree> AbstractSyntaxTreeBuilder::build() {
	return std::make_unique<AbstractSyntaxTree>(treeBuilderContext.popTranslationUnit());
}

} // namespace ast
