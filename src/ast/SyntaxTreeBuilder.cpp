#include "SyntaxTreeBuilder.h"

#include "AbstractSyntaxTree.h"
#include "Block.h"
#include "Expression.h"
#include "GnuExtensions.h"
#include "parser/ParseExtensions.h"
#include "util/Diagnostic.h"

#include <stdexcept>

namespace ast {

std::unique_ptr<SyntaxTreeBuilder> SyntaxTreeBuilder::create(
        const parser::Grammar* grammar, scanner::LexicalSession& session, bool gnuExtensions) {
    std::unique_ptr<parser::ParseExtensions> extensions;
    if (gnuExtensions) {
        auto gnu = std::make_unique<GnuExtensions>();
        gnu->installTypes(session);
        extensions = std::move(gnu);
    }
    return std::make_unique<SyntaxTreeBuilder>(
            grammar, session, std::move(extensions), gnuExtensions);
}

SyntaxTreeBuilder::SyntaxTreeBuilder(const parser::Grammar* grammar, scanner::LexicalSession& session,
        std::unique_ptr<parser::ParseExtensions> extensions, bool gnuExtensions):
    syntaxNodeBuilder{*grammar},
    treeBuilderContext{session},
    extensions_ { std::move(extensions) }
{
    treeBuilderContext.environment().setGnuExtensions(gnuExtensions);
}

SyntaxTreeBuilder::SyntaxTreeBuilder(const parser::Grammar* grammar,
        SyntaxTreeBuilder& parent) :
    SyntaxTreeBuilder(grammar, parent.session(), parent.environment())
{
    if (parent.hasSink()) {
        setSink(&parent.sink());
    }
}

SyntaxTreeBuilder::SyntaxTreeBuilder(const parser::Grammar* grammar, scanner::LexicalSession& session,
        ParseEnvironment& parentEnvironment):
    syntaxNodeBuilder{*grammar},
    treeBuilderContext{session, parentEnvironment}
{
}

SyntaxTreeBuilder::~SyntaxTreeBuilder() = default;

void SyntaxTreeBuilder::makeNonterminalNode(const parser::Production& production) {
	syntaxNodeBuilder.updateContext(production, treeBuilderContext);
}

void SyntaxTreeBuilder::makeTerminalNode(std::string value, const translation_unit::Context& context) {
	treeBuilderContext.pushTerminal( { std::move(value), context });
}

parser::ParseExtensions* SyntaxTreeBuilder::parseExtensions() {
    return extensions_.get();
}

void SyntaxTreeBuilder::setSink(diag::Sink* sink) {
    sink_ = sink;
    treeBuilderContext.setSink(sink);
}

diag::Sink& SyntaxTreeBuilder::sink() const {
    if (!sink_) {
        throw std::logic_error { "missing diagnostic sink" };
    }
    return *sink_;
}

bool SyntaxTreeBuilder::aborted() const {
    return hasError() || treeBuilderContext.failed();
}

void SyntaxTreeBuilder::assertBuildable() const {
    if (erred_) {
        throw std::runtime_error { "parsing failed with syntax errors" };
    }
}

scanner::LexicalSession& SyntaxTreeBuilder::session() {
    return treeBuilderContext.environment().session();
}

ParseEnvironment& SyntaxTreeBuilder::environment() {
    return treeBuilderContext.environment();
}

void SyntaxTreeBuilder::pushExpression(std::unique_ptr<Expression> expression) {
    treeBuilderContext.pushExpression(std::move(expression));
}

void SyntaxTreeBuilder::pushTypeSpecifier(TypeSpecifier typeSpecifier) {
    treeBuilderContext.pushTypeSpecifier(std::move(typeSpecifier));
}

std::unique_ptr<Block> SyntaxTreeBuilder::popBlock() {
    return treeBuilderContext.popBlock();
}

std::unique_ptr<Expression> SyntaxTreeBuilder::takeExpression() {
    return treeBuilderContext.popExpression();
}

std::optional<TypeSpecifier> SyntaxTreeBuilder::takeTypeSpecifier() {
    if (!treeBuilderContext.hasTypeSpecifier()) {
        return std::nullopt;
    }
    return treeBuilderContext.popTypeSpecifier();
}

std::unique_ptr<AbstractSyntaxTree> SyntaxTreeBuilder::buildTree() {
    assertBuildable();
    auto tree = std::make_unique<AbstractSyntaxTree>(treeBuilderContext.popTranslationUnit());
    tree->setVlaExpressions(treeBuilderContext.environment().vlaExpressionsShared());
    return tree;
}

} // namespace ast
