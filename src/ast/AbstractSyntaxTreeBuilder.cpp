#include "AbstractSyntaxTreeBuilder.h"

#include "AbstractSyntaxTree.h"
#include "Block.h"
#include "Expression.h"
#include "parser/ParseExtensions.h"

namespace ast {

AbstractSyntaxTreeBuilder::AbstractSyntaxTreeBuilder(const parser::Grammar* grammar, scanner::LexicalSession& session,
        std::unique_ptr<parser::ParseExtensions> extensions, bool gnuExtensions):
    syntaxNodeBuilder{*grammar},
    treeBuilderContext{session},
    extensions_ { std::move(extensions) }
{
    treeBuilderContext.environment().setGnuExtensions(gnuExtensions);
}

AbstractSyntaxTreeBuilder::AbstractSyntaxTreeBuilder(const parser::Grammar* grammar, scanner::LexicalSession& session,
        ParseEnvironment& parentEnvironment):
    syntaxNodeBuilder{*grammar},
    treeBuilderContext{session, parentEnvironment}
{
}

AbstractSyntaxTreeBuilder::~AbstractSyntaxTreeBuilder() = default;

void AbstractSyntaxTreeBuilder::makeNonterminalNode(const parser::Production& production) {
	syntaxNodeBuilder.updateContext(production, treeBuilderContext);
}

void AbstractSyntaxTreeBuilder::makeTerminalNode(std::string type, std::string value, const translation_unit::Context& context) {
	treeBuilderContext.pushTerminal( { type, value, context });
}

parser::ParseExtensions* AbstractSyntaxTreeBuilder::parseExtensions() {
    return extensions_.get();
}

scanner::LexicalSession& AbstractSyntaxTreeBuilder::session() {
    return treeBuilderContext.environment().session();
}

ParseEnvironment& AbstractSyntaxTreeBuilder::environment() {
    return treeBuilderContext.environment();
}

void AbstractSyntaxTreeBuilder::pushExpression(std::unique_ptr<Expression> expression) {
    treeBuilderContext.pushExpression(std::move(expression));
}

void AbstractSyntaxTreeBuilder::pushTypeSpecifier(TypeSpecifier typeSpecifier) {
    treeBuilderContext.pushTypeSpecifier(std::move(typeSpecifier));
}

std::unique_ptr<Block> AbstractSyntaxTreeBuilder::takeCompoundBlock() {
    auto node = treeBuilderContext.popStatement();
    auto* block = dynamic_cast<Block*>(node.get());
    if (!block) {
        return nullptr;
    }
    node.release();
    return std::unique_ptr<Block> { block };
}

std::unique_ptr<Expression> AbstractSyntaxTreeBuilder::takeExpression() {
    return treeBuilderContext.popExpression();
}

std::optional<TypeSpecifier> AbstractSyntaxTreeBuilder::takeTypeSpecifier() {
    if (!treeBuilderContext.hasTypeSpecifier()) {
        return std::nullopt;
    }
    return treeBuilderContext.popTypeSpecifier();
}

std::optional<TypeName> AbstractSyntaxTreeBuilder::takeTypeName() {
    if (!treeBuilderContext.hasTypeName()) {
        return std::nullopt;
    }
    return treeBuilderContext.popTypeName();
}

std::unique_ptr<parser::SyntaxTree> AbstractSyntaxTreeBuilder::build() {
    assertBuildable();
    auto tree = std::make_unique<AbstractSyntaxTree>(
            treeBuilderContext.popTranslationUnit());
    tree->setParseEnumConstants(treeBuilderContext.environment().enumConstantsSnapshot());
    return tree;
}

} // namespace ast
