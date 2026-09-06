#include "gtest/gtest.h"

#include <memory>

#include "ast/AbstractSyntaxTreeBuilderContext.h"
#include "ast/Block.h"
#include "ast/CSNB_Internal.h"
#include "ast/Declaration.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/ExpressionStatement.h"
#include "ast/IdentifierExpression.h"
#include "ast/IfStatement.h"
#include "ast/NullStatement.h"
#include "ast/TypeSpecifier.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"

namespace {

translation_unit::Context ctx() {
    return { "t", 1 };
}

ast::DeclarationSpecifiers intSpecs() {
    return ast::DeclarationSpecifiers { ast::TypeSpecifier { type::signedInteger(), "int" } };
}

TEST(Statement, expressionStatementWrapsExpression) {
    auto expression = std::make_unique<ast::IdentifierExpression>("x", ctx());
    auto* raw = expression.get();
    ast::ExpressionStatement statement { std::move(expression) };
    EXPECT_EQ(statement.nodeKind(), ast::NodeKind::ExpressionStatement);
    EXPECT_EQ(statement.expression.get(), raw);
}

TEST(Statement, ifBodyIsAStatement) {
    ast::IfStatement statement {
            std::make_unique<ast::IdentifierExpression>("x", ctx()),
            std::make_unique<ast::NullStatement>() };
    EXPECT_EQ(statement.body->nodeKind(), ast::NodeKind::NullStatement);
}

TEST(BuilderContext, popAsStatementPassesBlockThrough) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };
    context.pushStatement(std::make_unique<ast::Block>());
    auto statement = context.popAsStatement();
    ASSERT_NE(statement, nullptr);
    EXPECT_EQ(statement->nodeKind(), ast::NodeKind::Block);
}

TEST(BuilderContext, popAsStatementWrapsExpression) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };
    auto expression = std::make_unique<ast::IdentifierExpression>("x", ctx());
    auto* raw = expression.get();
    context.pushStatement(std::move(expression));
    auto statement = context.popAsStatement();
    ASSERT_NE(statement, nullptr);
    EXPECT_EQ(statement->nodeKind(), ast::NodeKind::ExpressionStatement);
    auto* wrapped = static_cast<ast::ExpressionStatement*>(statement.get());
    EXPECT_EQ(wrapped->expression.get(), raw);
}

TEST(BuilderContext, popAsStatementRejectsDeclaration) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };
    context.pushStatement(std::make_unique<ast::Declaration>(intSpecs()));
    EXPECT_EQ(context.popAsStatement(), nullptr);
}

TEST(CSNBCreators, emptyStatementPushesNullStatement) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };
    context.pushTerminal({ ";", ctx() });
    ast::emptyStatement(context);
    auto statement = context.popAsStatement();
    ASSERT_NE(statement, nullptr);
    EXPECT_EQ(statement->nodeKind(), ast::NodeKind::NullStatement);
}

} // namespace
