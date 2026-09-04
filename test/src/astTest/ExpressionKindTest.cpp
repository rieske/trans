#include "gtest/gtest.h"

#include <memory>
#include <vector>

#include "ast/ArithmeticExpression.h"
#include "ast/ArrayAccess.h"
#include "ast/AssignmentExpression.h"
#include "ast/BitwiseExpression.h"
#include "ast/Block.h"
#include "ast/ComparisonExpression.h"
#include "ast/CompoundLiteral.h"
#include "ast/ConditionalExpression.h"
#include "ast/Constant.h"
#include "ast/ConstantExpression.h"
#include "ast/ExpressionList.h"
#include "ast/FunctionCall.h"
#include "ast/GenericSelection.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializerListExpression.h"
#include "ast/LogicalAndExpression.h"
#include "ast/LogicalOrExpression.h"
#include "ast/MemberAccess.h"
#include "ast/PostfixExpression.h"
#include "ast/PrefixExpression.h"
#include "ast/ShiftExpression.h"
#include "ast/StatementExpression.h"
#include "ast/StringLiteralExpression.h"
#include "ast/TypeCast.h"
#include "ast/TypeNameExpression.h"
#include "ast/TypeSpecifier.h"
#include "ast/UnaryExpression.h"
#include "translation_unit/Context.h"
#include "types/Type.h"

namespace {

translation_unit::Context ctx() {
    return { "t", 1 };
}

std::unique_ptr<ast::IdentifierExpression> id(const char* name = "x") {
    return std::make_unique<ast::IdentifierExpression>(name, ctx());
}

ast::TypeSpecifier intSpec() {
    return { type::signedInteger(), "int" };
}

// No default: a new enumerator is a -Wswitch/-Werror failure.
std::unique_ptr<ast::Expression> makeLeaf(ast::ExprKind kind) {
    switch (kind) {
    case ast::ExprKind::Identifier:
        return std::make_unique<ast::IdentifierExpression>("x", ctx());
    case ast::ExprKind::Constant:
        return std::make_unique<ast::ConstantExpression>(
                ast::Constant("1", type::signedInteger(), ctx()));
    case ast::ExprKind::StringLiteral:
        return std::make_unique<ast::StringLiteralExpression>("\"hi\"", ctx());
    case ast::ExprKind::InitList:
        return std::make_unique<ast::InitializerListExpression>(
                std::vector<ast::InitializerElement> { });
    case ast::ExprKind::ArrayAccess:
        return std::make_unique<ast::ArrayAccess>(id(), id("i"));
    case ast::ExprKind::MemberAccess:
        return std::make_unique<ast::MemberAccess>(id(), "f", false, ctx());
    case ast::ExprKind::FunctionCall:
        return std::make_unique<ast::FunctionCall>(id("f"));
    case ast::ExprKind::Prefix:
        return std::make_unique<ast::PrefixExpression>("++", id());
    case ast::ExprKind::Postfix:
        return std::make_unique<ast::PostfixExpression>(id(), "++");
    case ast::ExprKind::Unary:
        return std::make_unique<ast::UnaryExpression>("-", id());
    case ast::ExprKind::TypeCast:
        return std::make_unique<ast::TypeCast>(intSpec(), id());
    case ast::ExprKind::Arithmetic:
        return std::make_unique<ast::ArithmeticExpression>(id(), "+", id());
    case ast::ExprKind::Shift:
        return std::make_unique<ast::ShiftExpression>(id(), "<<", id());
    case ast::ExprKind::Comparison:
        return std::make_unique<ast::ComparisonExpression>(id(), "<", id());
    case ast::ExprKind::Bitwise:
        return std::make_unique<ast::BitwiseExpression>(id(), "&", id());
    case ast::ExprKind::LogicalAnd:
        return std::make_unique<ast::LogicalAndExpression>(id(), id());
    case ast::ExprKind::LogicalOr:
        return std::make_unique<ast::LogicalOrExpression>(id(), id());
    case ast::ExprKind::Conditional:
        return std::make_unique<ast::ConditionalExpression>(id("c"), id("t"), id("f"));
    case ast::ExprKind::Assignment:
        return std::make_unique<ast::AssignmentExpression>(id(), "=", id());
    case ast::ExprKind::Comma:
        return std::make_unique<ast::ExpressionList>(id(), id());
    case ast::ExprKind::TypeName:
        return std::make_unique<ast::TypeNameExpression>(intSpec(), ctx());
    case ast::ExprKind::CompoundLiteral:
        return std::make_unique<ast::CompoundLiteral>(intSpec(),
                std::make_unique<ast::InitializerListExpression>(
                        std::vector<ast::InitializerElement> { }));
    case ast::ExprKind::GenericSelection: {
        std::vector<ast::GenericAssociation> associations;
        associations.push_back({ std::nullopt, id("y") });
        return std::make_unique<ast::GenericSelection>(ctx(), id("c"), std::move(associations));
    }
    case ast::ExprKind::StatementExpression:
        return std::make_unique<ast::StatementExpression>(ctx(),
                std::make_unique<ast::Block>(
                        std::vector<std::unique_ptr<ast::AbstractSyntaxTreeNode>> { }));
    }
    return nullptr;
}

TEST(ExpressionKind, everyLeafReportsItsKind) {
    ASSERT_EQ(static_cast<int>(ast::ExprKind::Identifier), 0);
    const int last = static_cast<int>(ast::ExprKind::StatementExpression);
    for (int i = 0; i <= last; ++i) {
        const auto kind = static_cast<ast::ExprKind>(i);
        auto node = makeLeaf(kind);
        ASSERT_NE(node, nullptr) << i;
        EXPECT_EQ(node->exprKind(), kind);
    }
}

TEST(ExpressionKind, asInitListAndAsStringLiteral) {
    ast::IdentifierExpression identifier("x", ctx());
    EXPECT_EQ(identifier.asInitList(), nullptr);
    EXPECT_EQ(identifier.asStringLiteral(), nullptr);

    ast::InitializerListExpression list { std::vector<ast::InitializerElement> { } };
    EXPECT_EQ(list.asInitList(), &list);
    EXPECT_EQ(list.asStringLiteral(), nullptr);
    const ast::Expression& listAsExpr = list;
    EXPECT_EQ(listAsExpr.asInitList(), &list);
    EXPECT_EQ(listAsExpr.asStringLiteral(), nullptr);

    ast::StringLiteralExpression str("\"hi\"", ctx());
    EXPECT_EQ(str.asStringLiteral(), &str);
    EXPECT_EQ(str.asInitList(), nullptr);
    const ast::Expression& strAsExpr = str;
    EXPECT_EQ(strAsExpr.asStringLiteral(), &str);
    EXPECT_EQ(strAsExpr.asInitList(), nullptr);
}

} // namespace
