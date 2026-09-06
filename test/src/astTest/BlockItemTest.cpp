#include "gtest/gtest.h"

#include <memory>
#include <vector>

#include "ast/Block.h"
#include "ast/Declaration.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/IdentifierExpression.h"
#include "ast/TypeSpecifier.h"
#include "ast/VoidReturnStatement.h"
#include "types/Type.h"

namespace {

translation_unit::Context ctx() {
    return { "t", 1 };
}

ast::DeclarationSpecifiers intSpecs() {
    return ast::DeclarationSpecifiers { ast::TypeSpecifier { type::signedInteger(), "int" } };
}

TEST(BlockItem, holdsDeclaration) {
    auto declaration = std::make_unique<ast::Declaration>(intSpecs());
    auto* raw = declaration.get();
    ast::BlockItem item { std::move(declaration) };
    EXPECT_EQ(item.asDeclaration(), raw);
    EXPECT_EQ(item.asExpression(), nullptr);
    EXPECT_EQ(item.asStatement(), nullptr);
    const ast::BlockItem& asConst = item;
    EXPECT_EQ(asConst.asDeclaration(), raw);
    EXPECT_EQ(asConst.asExpression(), nullptr);
}

TEST(BlockItem, holdsExpression) {
    auto expression = std::make_unique<ast::IdentifierExpression>("x", ctx());
    auto* raw = expression.get();
    ast::BlockItem item { std::move(expression) };
    EXPECT_EQ(item.asExpression(), raw);
    EXPECT_EQ(item.asDeclaration(), nullptr);
    EXPECT_EQ(item.asStatement(), nullptr);
    const ast::BlockItem& asConst = item;
    EXPECT_EQ(asConst.asExpression(), raw);
    EXPECT_EQ(asConst.asDeclaration(), nullptr);
}

TEST(BlockItem, takeBlockTakesBlock) {
    auto block = std::make_unique<ast::Block>();
    auto* raw = block.get();
    ast::BlockItem item { std::move(block) };
    auto taken = item.takeBlock();
    ASSERT_NE(taken, nullptr);
    EXPECT_EQ(taken.get(), raw);
}

TEST(BlockItem, takeBlockRejectsNonBlock) {
    ast::BlockItem statement { std::make_unique<ast::VoidReturnStatement>() };
    EXPECT_EQ(statement.takeBlock(), nullptr);
    ast::BlockItem declaration { std::make_unique<ast::Declaration>(intSpecs()) };
    EXPECT_EQ(declaration.takeBlock(), nullptr);
    ast::BlockItem expression { std::make_unique<ast::IdentifierExpression>("x", ctx()) };
    EXPECT_EQ(expression.takeBlock(), nullptr);
}

TEST(BlockItem, holdsStatement) {
    auto statement = std::make_unique<ast::VoidReturnStatement>();
    auto* raw = statement.get();
    ast::BlockItem item { std::move(statement) };
    EXPECT_EQ(item.asDeclaration(), nullptr);
    EXPECT_EQ(item.asExpression(), nullptr);
    EXPECT_EQ(item.asStatement(), raw);
    const ast::BlockItem& asConst = item;
    EXPECT_EQ(asConst.asStatement(), raw);
}

TEST(BlockItem, blockHoldsMixedItemsInOrder) {
    auto declaration = std::make_unique<ast::Declaration>(intSpecs());
    auto* decl = declaration.get();
    auto expression = std::make_unique<ast::IdentifierExpression>("x", ctx());
    auto* expr = expression.get();

    std::vector<ast::BlockItem> items;
    items.push_back(ast::BlockItem { std::move(declaration) });
    items.push_back(ast::BlockItem { std::make_unique<ast::VoidReturnStatement>() });
    items.push_back(ast::BlockItem { std::move(expression) });
    ast::Block block { std::move(items) };

    ASSERT_EQ(block.getItems().size(), 3u);
    EXPECT_EQ(block.getItems()[0].asDeclaration(), decl);
    EXPECT_EQ(block.getItems()[1].asDeclaration(), nullptr);
    EXPECT_EQ(block.getItems()[1].asExpression(), nullptr);
    EXPECT_EQ(block.getItems()[2].asExpression(), expr);
}

} // namespace
