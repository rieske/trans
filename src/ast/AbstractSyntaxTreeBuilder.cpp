#include "AbstractSyntaxTreeBuilder.h"

#include "AbstractSyntaxTree.h"
#include "AbstractSyntaxTreeNode.h"
#include "TerminalSymbol.h"
#include "ast/ContextualSyntaxNodeBuilder.h"
#include "scanner/FiniteAutomaton.h"

namespace ast {

AbstractSyntaxTreeBuilder::AbstractSyntaxTreeBuilder(const parser::Grammar* grammar):
    syntaxNodeBuilder{*grammar}
{
    // Fresh typedef namespace for each compilation unit.
    scanner::FiniteAutomaton::clearTypedefNames();
}

AbstractSyntaxTreeBuilder::~AbstractSyntaxTreeBuilder() = default;

void AbstractSyntaxTreeBuilder::makeNonterminalNode(const parser::Production& production) {
	syntaxNodeBuilder.updateContext(production, treeBuilderContext);
}

void AbstractSyntaxTreeBuilder::makeTerminalNode(std::string type, std::string value, const translation_unit::Context& context) {
	treeBuilderContext.pushTerminal( { type, value, context });
}

std::unique_ptr<parser::SyntaxTree> AbstractSyntaxTreeBuilder::build() {
    assertBuildable();
	return std::make_unique<AbstractSyntaxTree>(
            treeBuilderContext.popTranslationUnit(),
            treeBuilderContext.environment().takePendingArrayMembers());
}

} // namespace ast
