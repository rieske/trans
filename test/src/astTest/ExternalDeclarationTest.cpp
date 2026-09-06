#include "gtest/gtest.h"

#include <memory>
#include <vector>

#include "ast/AbstractSyntaxTree.h"
#include "ast/Block.h"
#include "ast/Declaration.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/Declarator.h"
#include "ast/ExternalDeclaration.h"
#include "ast/FunctionDefinition.h"
#include "ast/Identifier.h"
#include "ast/IdentifierExpression.h"
#include "ast/TerminalSymbol.h"
#include "ast/TypeSpecifier.h"
#include "types/Type.h"

namespace {

translation_unit::Context ctx() {
    return { "t", 1 };
}

ast::DeclarationSpecifiers intSpecs() {
    return ast::DeclarationSpecifiers { ast::TypeSpecifier { type::signedInteger(), "int" } };
}

std::unique_ptr<ast::Declarator> simpleDeclarator() {
    return std::make_unique<ast::Declarator>(
            std::make_unique<ast::Identifier>(ast::TerminalSymbol { "f", ctx() }));
}

TEST(ExternalDeclaration, fromNodeClassifiesDeclaration) {
    auto declaration = std::make_unique<ast::Declaration>(intSpecs());
    auto* raw = declaration.get();
    ast::ExternalDeclaration item = ast::ExternalDeclaration::fromNode(std::move(declaration));
    EXPECT_EQ(item.asDeclaration(), raw);
    EXPECT_EQ(item.asFunctionDefinition(), nullptr);
    const ast::ExternalDeclaration& asConst = item;
    EXPECT_EQ(asConst.asDeclaration(), raw);
    EXPECT_EQ(asConst.asFunctionDefinition(), nullptr);
}

TEST(ExternalDeclaration, fromNodeClassifiesFunctionDefinition) {
    auto function = std::make_unique<ast::FunctionDefinition>(
            intSpecs(), simpleDeclarator(), std::make_unique<ast::Block>());
    auto* raw = function.get();
    ast::ExternalDeclaration item = ast::ExternalDeclaration::fromNode(std::move(function));
    EXPECT_EQ(item.asFunctionDefinition(), raw);
    EXPECT_EQ(item.asDeclaration(), nullptr);
    const ast::ExternalDeclaration& asConst = item;
    EXPECT_EQ(asConst.asFunctionDefinition(), raw);
    EXPECT_EQ(asConst.asDeclaration(), nullptr);
}

TEST(ExternalDeclaration, fromNodeRejectsNonExternal) {
    ast::ExternalDeclaration item = ast::ExternalDeclaration::fromNode(
            std::make_unique<ast::IdentifierExpression>("x", ctx()));
    EXPECT_EQ(item.asDeclaration(), nullptr);
    EXPECT_EQ(item.asFunctionDefinition(), nullptr);
}

TEST(ExternalDeclaration, treeHoldsMixedItemsInOrder) {
    auto declaration = std::make_unique<ast::Declaration>(intSpecs());
    auto* decl = declaration.get();
    auto function = std::make_unique<ast::FunctionDefinition>(
            intSpecs(), simpleDeclarator(), std::make_unique<ast::Block>());
    auto* fn = function.get();

    std::vector<ast::ExternalDeclaration> items;
    items.push_back(ast::ExternalDeclaration { std::move(declaration) });
    items.push_back(ast::ExternalDeclaration { std::move(function) });
    ast::AbstractSyntaxTree tree { std::move(items) };

    auto it = tree.begin();
    ASSERT_NE(it, tree.end());
    EXPECT_EQ(it->asDeclaration(), decl);
    EXPECT_EQ(it->asFunctionDefinition(), nullptr);
    ++it;
    ASSERT_NE(it, tree.end());
    EXPECT_EQ(it->asFunctionDefinition(), fn);
    EXPECT_EQ(it->asDeclaration(), nullptr);
    ++it;
    EXPECT_EQ(it, tree.end());
}

} // namespace
