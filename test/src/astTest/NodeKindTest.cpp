#include "gtest/gtest.h"

#include <memory>
#include <vector>

#include "ast/Block.h"
#include "ast/CaseLabel.h"
#include "ast/Declaration.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/Declarator.h"
#include "ast/DefaultLabel.h"
#include "ast/FormalArgument.h"
#include "ast/FunctionDefinition.h"
#include "ast/GotoStatement.h"
#include "ast/Identifier.h"
#include "ast/IdentifierExpression.h"
#include "ast/IfElseStatement.h"
#include "ast/IfStatement.h"
#include "ast/InitializedDeclarator.h"
#include "ast/JumpStatement.h"
#include "ast/LabeledStatement.h"
#include "ast/LoopStatement.h"
#include "ast/Pointer.h"
#include "ast/ReturnStatement.h"
#include "ast/SwitchStatement.h"
#include "ast/TerminalSymbol.h"
#include "ast/TypeSpecifier.h"
#include "ast/ExpressionStatement.h"
#include "ast/NullStatement.h"
#include "ast/VoidReturnStatement.h"
#include "ast/WhileLoopHeader.h"
#include "types/Type.h"

namespace {

translation_unit::Context ctx() {
    return { "t", 1 };
}

ast::TerminalSymbol term(const char* value = "x") {
    return { value, ctx() };
}

std::unique_ptr<ast::IdentifierExpression> idExpr(const char* name = "x") {
    return std::make_unique<ast::IdentifierExpression>(name, ctx());
}

std::unique_ptr<ast::Block> emptyBlock() {
    return std::make_unique<ast::Block>();
}

std::unique_ptr<ast::VoidReturnStatement> emptyStmt() {
    return std::make_unique<ast::VoidReturnStatement>();
}

ast::DeclarationSpecifiers intSpecs() {
    return ast::DeclarationSpecifiers { ast::TypeSpecifier { type::signedInteger(), "int" } };
}

std::unique_ptr<ast::Declarator> simpleDeclarator() {
    return std::make_unique<ast::Declarator>(std::make_unique<ast::Identifier>(term()));
}

// No default: a new enumerator is a -Wswitch/-Werror failure.
std::unique_ptr<ast::AbstractSyntaxTreeNode> makeNode(ast::NodeKind kind) {
    switch (kind) {
    case ast::NodeKind::Expression:
        return idExpr();
    case ast::NodeKind::Declaration:
        return std::make_unique<ast::Declaration>(intSpecs());
    case ast::NodeKind::Block:
        return emptyBlock();
    case ast::NodeKind::FunctionDefinition:
        return std::make_unique<ast::FunctionDefinition>(intSpecs(), simpleDeclarator(), emptyBlock());
    case ast::NodeKind::IfStatement:
        return std::make_unique<ast::IfStatement>(idExpr(), emptyStmt());
    case ast::NodeKind::IfElseStatement:
        return std::make_unique<ast::IfElseStatement>(idExpr(), emptyStmt(), emptyStmt());
    case ast::NodeKind::LoopStatement:
        return std::make_unique<ast::LoopStatement>(
                std::make_unique<ast::WhileLoopHeader>(idExpr()), emptyStmt());
    case ast::NodeKind::SwitchStatement:
        return std::make_unique<ast::SwitchStatement>(idExpr(), emptyStmt());
    case ast::NodeKind::LabeledStatement:
        return std::make_unique<ast::LabeledStatement>(term("L"), emptyStmt());
    case ast::NodeKind::CaseLabel:
        return std::make_unique<ast::CaseLabel>(idExpr(), emptyStmt());
    case ast::NodeKind::DefaultLabel:
        return std::make_unique<ast::DefaultLabel>(term("default"), emptyStmt());
    case ast::NodeKind::JumpStatement:
        return std::make_unique<ast::JumpStatement>(ast::TerminalSymbol { "break", ctx() });
    case ast::NodeKind::GotoStatement:
        return std::make_unique<ast::GotoStatement>(term("goto"), term("L"));
    case ast::NodeKind::ReturnStatement:
        return std::make_unique<ast::ReturnStatement>(idExpr());
    case ast::NodeKind::VoidReturnStatement:
        return emptyStmt();
    case ast::NodeKind::ExpressionStatement:
        return std::make_unique<ast::ExpressionStatement>(idExpr());
    case ast::NodeKind::NullStatement:
        return std::make_unique<ast::NullStatement>();
    case ast::NodeKind::DeclarationSpecifiers:
        return std::make_unique<ast::DeclarationSpecifiers>(intSpecs());
    case ast::NodeKind::Declarator:
        return simpleDeclarator();
    case ast::NodeKind::DirectDeclarator:
        return std::make_unique<ast::Identifier>(term());
    case ast::NodeKind::Pointer:
        return std::make_unique<ast::Pointer>();
    case ast::NodeKind::FormalArgument:
        return std::make_unique<ast::FormalArgument>(intSpecs());
    case ast::NodeKind::InitializedDeclarator:
        return std::make_unique<ast::InitializedDeclarator>(simpleDeclarator());
    case ast::NodeKind::LoopHeader:
        return std::make_unique<ast::WhileLoopHeader>(idExpr());
    }
    return nullptr;
}

// No default: a new enumerator is a -Wswitch/-Werror failure.
bool isStatementKind(ast::NodeKind kind) {
    switch (kind) {
    case ast::NodeKind::Block:
    case ast::NodeKind::IfStatement:
    case ast::NodeKind::IfElseStatement:
    case ast::NodeKind::LoopStatement:
    case ast::NodeKind::SwitchStatement:
    case ast::NodeKind::LabeledStatement:
    case ast::NodeKind::CaseLabel:
    case ast::NodeKind::DefaultLabel:
    case ast::NodeKind::JumpStatement:
    case ast::NodeKind::GotoStatement:
    case ast::NodeKind::ReturnStatement:
    case ast::NodeKind::VoidReturnStatement:
    case ast::NodeKind::ExpressionStatement:
    case ast::NodeKind::NullStatement:
        return true;
    case ast::NodeKind::Expression:
    case ast::NodeKind::Declaration:
    case ast::NodeKind::FunctionDefinition:
    case ast::NodeKind::DeclarationSpecifiers:
    case ast::NodeKind::Declarator:
    case ast::NodeKind::DirectDeclarator:
    case ast::NodeKind::Pointer:
    case ast::NodeKind::FormalArgument:
    case ast::NodeKind::InitializedDeclarator:
    case ast::NodeKind::LoopHeader:
        return false;
    }
    return false;
}

TEST(NodeKind, everyConcreteReportsItsKind) {
    ASSERT_EQ(static_cast<int>(ast::NodeKind::Expression), 0);
    const int last = static_cast<int>(ast::NodeKind::LoopHeader);
    for (int i = 0; i <= last; ++i) {
        const auto kind = static_cast<ast::NodeKind>(i);
        auto node = makeNode(kind);
        ASSERT_NE(node, nullptr) << i;
        EXPECT_EQ(node->nodeKind(), kind);
        if (isStatementKind(kind)) {
            EXPECT_EQ(node->asStatement(), node.get());
        } else {
            EXPECT_EQ(node->asStatement(), nullptr);
        }
    }
}

TEST(NodeKind, typedAccessors) {
    ast::IdentifierExpression identifier("x", ctx());
    EXPECT_EQ(identifier.asExpression(), &identifier);
    EXPECT_EQ(identifier.asDeclaration(), nullptr);
    EXPECT_EQ(identifier.asBlock(), nullptr);
    const ast::AbstractSyntaxTreeNode& identifierAsNode = identifier;
    EXPECT_EQ(identifierAsNode.asExpression(), &identifier);
    EXPECT_EQ(identifierAsNode.asDeclaration(), nullptr);

    ast::Declaration declaration { intSpecs() };
    EXPECT_EQ(declaration.asDeclaration(), &declaration);
    EXPECT_EQ(declaration.asFunctionDefinition(), nullptr);
    EXPECT_EQ(declaration.asExpression(), nullptr);
    EXPECT_EQ(declaration.asBlock(), nullptr);
    const ast::AbstractSyntaxTreeNode& declarationAsNode = declaration;
    EXPECT_EQ(declarationAsNode.asDeclaration(), &declaration);
    EXPECT_EQ(declarationAsNode.asFunctionDefinition(), nullptr);
    EXPECT_EQ(declarationAsNode.asExpression(), nullptr);

    ast::FunctionDefinition function { intSpecs(), simpleDeclarator(), emptyBlock() };
    EXPECT_EQ(function.asFunctionDefinition(), &function);
    EXPECT_EQ(function.asDeclaration(), nullptr);
    EXPECT_EQ(function.asStatement(), nullptr);
    const ast::AbstractSyntaxTreeNode& functionAsNode = function;
    EXPECT_EQ(functionAsNode.asFunctionDefinition(), &function);
    EXPECT_EQ(functionAsNode.asDeclaration(), nullptr);

    ast::Block block;
    EXPECT_EQ(block.asBlock(), &block);
    EXPECT_EQ(block.asStatement(), &block);
    EXPECT_EQ(block.asExpression(), nullptr);
    EXPECT_EQ(block.asDeclaration(), nullptr);
    const ast::AbstractSyntaxTreeNode& blockAsNode = block;
    EXPECT_EQ(blockAsNode.asBlock(), &block);
    EXPECT_EQ(blockAsNode.asStatement(), &block);
    EXPECT_EQ(blockAsNode.asDeclaration(), nullptr);
}

} // namespace
