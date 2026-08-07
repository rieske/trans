#include "AbstractSyntaxTreeBuilder.h"

#include "AbstractSyntaxTree.h"
#include "AbstractSyntaxTreeNode.h"
#include "TerminalSymbol.h"
#include "ast/ContextualSyntaxNodeBuilder.h"

namespace ast {

AbstractSyntaxTreeBuilder::AbstractSyntaxTreeBuilder(const parser::Grammar* grammar, scanner::LexicalSession& session):
    syntaxNodeBuilder{*grammar},
    treeBuilderContext{session}
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
    assertBuildable();
    auto tree = std::make_unique<AbstractSyntaxTree>(
            treeBuilderContext.popTranslationUnit(),
            treeBuilderContext.environment().takePendingArrayMembers());
    // Sole enum handoff: session.enums snapshot for SA import (not TypeSpecifier lists).
    tree->setParseEnumConstants(treeBuilderContext.environment().enumConstantsSnapshot());
    return tree;
}

} // namespace ast
