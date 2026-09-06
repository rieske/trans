#include "gtest/gtest.h"

#include <memory>

#include "ast/Block.h"
#include "ast/Declaration.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/Declarator.h"
#include "ast/ExternalDeclaration.h"
#include "ast/FunctionDefinition.h"
#include "ast/Identifier.h"
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

TEST(ExternalDeclaration, holdsDeclaration) {
    auto declaration = std::make_unique<ast::Declaration>(intSpecs());
    auto* raw = declaration.get();
    ast::ExternalDeclaration item { std::move(declaration) };
    EXPECT_EQ(item.asDeclaration(), raw);
    EXPECT_EQ(item.asFunctionDefinition(), nullptr);
    const ast::ExternalDeclaration& asConst = item;
    EXPECT_EQ(asConst.asDeclaration(), raw);
    EXPECT_EQ(asConst.asFunctionDefinition(), nullptr);
}

TEST(ExternalDeclaration, holdsFunctionDefinition) {
    auto function = std::make_unique<ast::FunctionDefinition>(
            intSpecs(), simpleDeclarator(), std::make_unique<ast::Block>());
    auto* raw = function.get();
    ast::ExternalDeclaration item { std::move(function) };
    EXPECT_EQ(item.asFunctionDefinition(), raw);
    EXPECT_EQ(item.asDeclaration(), nullptr);
    const ast::ExternalDeclaration& asConst = item;
    EXPECT_EQ(asConst.asFunctionDefinition(), raw);
    EXPECT_EQ(asConst.asDeclaration(), nullptr);
}

} // namespace
