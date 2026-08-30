#include "gtest/gtest.h"

#include "ast/ArrayDeclarator.h"
#include "ast/ConstantExpression.h"
#include "ast/Declarator.h"
#include "ast/Identifier.h"
#include "ast/IdentifierExpression.h"
#include "ast/Operator.h"
#include "ast/ParseEnvironment.h"
#include "ast/UnaryExpression.h"
#include "ast/Pointer.h"
#include "ast/TerminalSymbol.h"
#include "ast/TypeSpecifier.h"
#include "ast/VlaExpressionTable.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"
#include "types/TypeQuery.h"

#include <memory>
#include <stdexcept>

namespace {

using namespace ast;

std::unique_ptr<Declarator> unnamedPointerDeclarator() {
    TerminalSymbol id { "id", "", { "t", 1 } };
    std::vector<Pointer> stars;
    stars.emplace_back();
    return std::make_unique<Declarator>(std::make_unique<Identifier>(id), std::move(stars));
}

TEST(TypeSpecifier, deferAbstractDeclaratorCombinesWhenTypeIsKnown) {
    TypeSpecifier ts { type::signedInteger(), "int" };
    ts.deferAbstractDeclarator(unnamedPointerDeclarator());
    EXPECT_TRUE(ts.getType().isPointer());
    EXPECT_TRUE(ts.getType().dereference().equivalentTo(type::signedInteger()));
    EXPECT_EQ(ts.getName(), "");
}

TEST(TypeSpecifier, pendingTypeofHasNoType) {
    TypeSpecifier ts { std::make_shared<IdentifierExpression>("x", translation_unit::Context { "t", 1 }) };
    EXPECT_FALSE(ts.hasType());
    EXPECT_TRUE(ts.needsSemanticResolve());
    EXPECT_THROW(ts.getType(), std::runtime_error);
    ts.deferAbstractDeclarator(unnamedPointerDeclarator());
    EXPECT_FALSE(ts.hasType());
    EXPECT_THROW(ts.getType(), std::runtime_error);
}

TEST(TypeSpecifier, dropSpellingKeepsTypeAndPendingDeclarator) {
    TypeSpecifier named { type::signedInteger(), "int" };
    named.dropSpelling();
    EXPECT_EQ(named.getName(), "");
    EXPECT_TRUE(named.getType().equivalentTo(type::signedInteger()));

    TypeSpecifier pending { std::make_shared<IdentifierExpression>("x", translation_unit::Context { "t", 1 }) };
    pending.deferAbstractDeclarator(unnamedPointerDeclarator());
    pending.dropSpelling();
    EXPECT_EQ(pending.getName(), "");
    EXPECT_FALSE(pending.hasType());
}

TEST(TypeSpecifier, unfixedArrayBoundLivesOnType) {
    translation_unit::Context ctx { "t", 1 };
    TypeSpecifier ts { type::signedInteger(), "int" };
    VlaExpressionTable table;
    auto array = std::make_unique<ArrayDeclarator>(
            std::make_unique<Identifier>(TerminalSymbol { "id", "", ctx }),
            std::make_unique<IdentifierExpression>("n", ctx),
            &table);
    Expression* bound = array->subscriptExpression.get();
    ts.deferAbstractDeclarator(std::make_unique<Declarator>(std::move(array)));
    EXPECT_TRUE(ts.getType().isVariableArray());
    EXPECT_EQ(table.require(ts.getType().vlaBound().get()).get(), bound);
    EXPECT_FALSE(type::hasUnspecifiedVlaSize(ts.getType()));
    EXPECT_TRUE(type::hasComputableRuntimeSize(ts.getType()));
}

TEST(ArrayDeclarator, unfixedBoundRequiresTable) {
    translation_unit::Context ctx { "t", 1 };
    EXPECT_THROW((ArrayDeclarator {
            std::make_unique<Identifier>(TerminalSymbol { "id", "", ctx }),
            std::make_unique<IdentifierExpression>("n", ctx) }), std::logic_error);
}

TEST(ArrayDeclarator, getFundamentalTypeReusesBoundIdentity) {
    translation_unit::Context ctx { "t", 1 };
    VlaExpressionTable table;
    Declarator declarator { std::make_unique<ArrayDeclarator>(
            std::make_unique<Identifier>(TerminalSymbol { "id", "a", ctx }),
            std::make_unique<IdentifierExpression>("n", ctx),
            &table) };
    auto first = declarator.getFundamentalType(type::signedInteger());
    auto second = declarator.getFundamentalType(type::signedInteger());
    EXPECT_EQ(first.vlaBound().get(), second.vlaBound().get());
    EXPECT_EQ(table.require(first.vlaBound().get()).get(),
            table.require(second.vlaBound().get()).get());
}

TEST(ArrayDeclarator, iceBoundDoesNotNeedTable) {
    translation_unit::Context ctx { "t", 1 };
    Declarator declarator { std::make_unique<ArrayDeclarator>(
            std::make_unique<Identifier>(TerminalSymbol { "id", "a", ctx }),
            std::make_unique<ConstantExpression>(
                    Constant { "3", type::signedInteger(), ctx })) };
    auto type = declarator.getFundamentalType(type::signedInteger());
    EXPECT_FALSE(type.isVariableArray());
    EXPECT_EQ(type.getArraySize(), 3);
}

TEST(ArrayDeclarator, missingSubscriptIsIncomplete) {
    translation_unit::Context ctx { "t", 1 };
    Declarator declarator { std::make_unique<ArrayDeclarator>(
            std::make_unique<Identifier>(TerminalSymbol { "id", "a", ctx }),
            nullptr) };
    auto type = declarator.getFundamentalType(type::signedInteger());
    EXPECT_TRUE(type.isIncompleteArray());
}

TEST(ArrayDeclarator, negativeIceIsZeroLengthShell) {
    translation_unit::Context ctx { "t", 1 };
    Declarator declarator { std::make_unique<ArrayDeclarator>(
            std::make_unique<Identifier>(TerminalSymbol { "id", "a", ctx }),
            std::make_unique<UnaryExpression>(
                    std::make_unique<Operator>("-"),
                    std::make_unique<ConstantExpression>(
                            Constant { "1", type::signedInteger(), ctx }))) };
    auto type = declarator.getFundamentalType(type::signedInteger());
    EXPECT_FALSE(type.isVariableArray());
    EXPECT_FALSE(type.isIncompleteArray());
    EXPECT_EQ(type.getArraySize(), 0);
}

TEST(TypeSpecifier, refoldTurnsIceBoundIntoConstantArray) {
    translation_unit::Context ctx { "t", 1 };
    auto three = std::make_shared<ConstantExpression>(
            Constant { "3", type::signedInteger(), ctx });
    VlaExpressionTable table;
    auto id = std::make_shared<type::VlaBound>();
    table.bind(id.get(), three);
    TypeSpecifier ts { type::variableArray(type::signedInteger(), id), "int" };
    EXPECT_TRUE(ts.getType().isVariableArray());
    ts.refoldConstantArrayBounds(table);
    EXPECT_FALSE(ts.getType().isVariableArray());
    EXPECT_EQ(ts.getType().getArraySize(), 3);
}

TEST(TypeSpecifier, applyDoesNotDoubleWrap) {
    TypeSpecifier ts { type::signedInteger(), "int" };
    ts.deferAbstractDeclarator(unnamedPointerDeclarator());
    EXPECT_TRUE(ts.getType().isPointer());
    EXPECT_FALSE(ts.getType().dereference().isPointer());
    EXPECT_FALSE(ts.needsSemanticResolve());

    scanner::LexicalSession session;
    ParseEnvironment env { session };
    ASSERT_TRUE(ts.resolveTypeofAtParseTime(env));
    EXPECT_TRUE(ts.getType().isPointer());
    EXPECT_FALSE(ts.getType().dereference().isPointer());
}

TEST(TypeSpecifier, resolveTypeofAtParseTimeUsesEnvironment) {
    scanner::LexicalSession session;
    ParseEnvironment env { session };
    env.defineObject("x", type::signedInteger());
    TypeSpecifier ts { std::make_shared<IdentifierExpression>("x", translation_unit::Context { "t", 1 }) };
    ts.deferAbstractDeclarator(unnamedPointerDeclarator());
    ASSERT_TRUE(ts.resolveTypeofAtParseTime(env));
    EXPECT_TRUE(ts.getType().isPointer());
    EXPECT_TRUE(ts.getType().dereference().equivalentTo(type::signedInteger()));
    EXPECT_FALSE(ts.needsSemanticResolve());

    TypeSpecifier unknown { std::make_shared<IdentifierExpression>("nope", translation_unit::Context { "t", 1 }) };
    EXPECT_FALSE(unknown.resolveTypeofAtParseTime(env));
    EXPECT_TRUE(unknown.needsSemanticResolve());
}

} // namespace
