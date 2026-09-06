#include "gtest/gtest.h"

#include <memory>

#include "ast/Declaration.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/ForInit.h"
#include "ast/ForLoopHeader.h"
#include "ast/IdentifierExpression.h"
#include "ast/TypeSpecifier.h"
#include "types/Type.h"

namespace {

translation_unit::Context ctx() {
    return { "t", 1 };
}

ast::DeclarationSpecifiers intSpecs() {
    return ast::DeclarationSpecifiers { ast::TypeSpecifier { type::signedInteger(), "int" } };
}

TEST(ForInit, absent) {
    ast::ForInit init;
    EXPECT_EQ(init.asDeclaration(), nullptr);
    EXPECT_EQ(init.asExpression(), nullptr);
}

TEST(ForInit, holdsDeclaration) {
    auto declaration = std::make_unique<ast::Declaration>(intSpecs());
    auto* raw = declaration.get();
    ast::ForInit init { std::move(declaration) };
    EXPECT_EQ(init.asDeclaration(), raw);
    EXPECT_EQ(init.asExpression(), nullptr);
}

TEST(ForInit, holdsExpression) {
    auto expression = std::make_unique<ast::IdentifierExpression>("x", ctx());
    auto* raw = expression.get();
    ast::ForInit init { std::move(expression) };
    EXPECT_EQ(init.asExpression(), raw);
    EXPECT_EQ(init.asDeclaration(), nullptr);
}

TEST(ForLoopHeader, declarationInitOpensBlockScope) {
    auto declaration = std::make_unique<ast::Declaration>(intSpecs());
    auto* raw = declaration.get();
    ast::ForLoopHeader header { ast::ForInit { std::move(declaration) }, nullptr, nullptr };
    EXPECT_TRUE(header.opensBlockScope());
    EXPECT_EQ(header.initialization.asDeclaration(), raw);
    EXPECT_EQ(header.initialization.asExpression(), nullptr);
}

TEST(ForLoopHeader, expressionInitDoesNotOpenBlockScope) {
    auto expression = std::make_unique<ast::IdentifierExpression>("x", ctx());
    auto* raw = expression.get();
    ast::ForLoopHeader header { ast::ForInit { std::move(expression) }, nullptr, nullptr };
    EXPECT_FALSE(header.opensBlockScope());
    EXPECT_EQ(header.initialization.asExpression(), raw);
    EXPECT_EQ(header.initialization.asDeclaration(), nullptr);
}

TEST(ForLoopHeader, absentInitDoesNotOpenBlockScope) {
    ast::ForLoopHeader header { ast::ForInit { }, nullptr, nullptr };
    EXPECT_FALSE(header.opensBlockScope());
    EXPECT_EQ(header.initialization.asDeclaration(), nullptr);
    EXPECT_EQ(header.initialization.asExpression(), nullptr);
}

} // namespace
