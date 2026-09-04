#include "AbstractSyntaxTreeBuilder.h"

#include "AbstractSyntaxTree.h"
#include "Block.h"
#include "Expression.h"
#include "GnuExtensions.h"
#include "parser/ParseExtensions.h"

namespace ast {

std::unique_ptr<AbstractSyntaxTreeBuilder> AbstractSyntaxTreeBuilder::create(
        const parser::Grammar* grammar, scanner::LexicalSession& session, bool gnuExtensions) {
    std::unique_ptr<parser::ParseExtensions> extensions;
    if (gnuExtensions) {
        auto gnu = std::make_unique<GnuExtensions>();
        gnu->installTypes(session);
        extensions = std::move(gnu);
    }
    return std::make_unique<AbstractSyntaxTreeBuilder>(
            grammar, session, std::move(extensions), gnuExtensions);
}

AbstractSyntaxTreeBuilder::AbstractSyntaxTreeBuilder(const parser::Grammar* grammar, scanner::LexicalSession& session,
        std::unique_ptr<parser::ParseExtensions> extensions, bool gnuExtensions):
    syntaxNodeBuilder{*grammar},
    treeBuilderContext{session},
    extensions_ { std::move(extensions) }
{
    treeBuilderContext.environment().setGnuExtensions(gnuExtensions);
}

AbstractSyntaxTreeBuilder::AbstractSyntaxTreeBuilder(const parser::Grammar* grammar,
        AbstractSyntaxTreeBuilder& parent) :
    AbstractSyntaxTreeBuilder(grammar, parent.session(), parent.environment())
{
    if (parent.hasSink()) {
        setSink(&parent.sink());
    }
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

void AbstractSyntaxTreeBuilder::setSink(diag::Sink* sink) {
    SyntaxTreeBuilder::setSink(sink);
    treeBuilderContext.setSink(sink);
}

bool AbstractSyntaxTreeBuilder::aborted() const {
    return hasError() || treeBuilderContext.failed();
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
    auto* block = node ? node->asBlock() : nullptr;
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

std::unique_ptr<AbstractSyntaxTree> AbstractSyntaxTreeBuilder::buildTree() {
    assertBuildable();
    auto tree = std::make_unique<AbstractSyntaxTree>(treeBuilderContext.popTranslationUnit());
    tree->setVlaExpressions(treeBuilderContext.environment().vlaExpressionsShared());
    return tree;
}

} // namespace ast
