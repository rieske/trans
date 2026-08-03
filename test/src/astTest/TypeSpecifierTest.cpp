#include "gtest/gtest.h"

#include "ast/ArrayDeclarator.h"
#include "ast/ConstantExpression.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/Declarator.h"
#include "ast/FormalArgument.h"
#include "ast/Identifier.h"
#include "ast/IdentifierExpression.h"
#include "ast/ParseEnvironment.h"
#include "ast/Pointer.h"
#include "ast/TerminalSymbol.h"
#include "ast/TypeName.h"
#include "ast/TypeSpecifier.h"
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

TypeName typeNameIntPointer() {
    return TypeName { TypeSpecifier { type::signedInteger(), "int" }, unnamedPointerDeclarator() };
}

TEST(TypeSpecifier, typeofTypeNameHoldsDadUntilResolve) {
    TypeSpecifier ts { typeNameIntPointer() };
    EXPECT_FALSE(ts.hasType());
    EXPECT_TRUE(ts.needsSemanticResolve());
    scanner::LexicalSession session;
    ParseEnvironment env { session };
    ASSERT_TRUE(ts.resolveTypeofAtParseTime(env));
    EXPECT_TRUE(ts.getType().isPointer());
    EXPECT_TRUE(ts.getType().dereference().equivalentTo(type::signedInteger()));
    EXPECT_FALSE(ts.needsSemanticResolve());
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
    auto array = std::make_unique<ArrayDeclarator>(
            std::make_unique<Identifier>(TerminalSymbol { "id", "", ctx }),
            std::make_unique<IdentifierExpression>("n", ctx));
    Expression* bound = array->subscriptExpression.get();
    ts.deferAbstractDeclarator(std::make_unique<Declarator>(std::move(array)));
    EXPECT_TRUE(ts.getType().isVariableArray());
    EXPECT_EQ(ts.getType().variableBound().get(), bound);
    EXPECT_FALSE(type::hasUnspecifiedVlaSize(ts.getType()));
    EXPECT_TRUE(type::hasComputableRuntimeSize(ts.getType()));
}

TEST(TypeSpecifier, refoldTurnsIceBoundIntoConstantArray) {
    translation_unit::Context ctx { "t", 1 };
    auto three = std::make_shared<ConstantExpression>(
            Constant { "3", type::signedInteger(), ctx });
    TypeSpecifier ts { type::variableArray(type::signedInteger(), three), "int" };
    EXPECT_TRUE(ts.getType().isVariableArray());
    ts.refoldConstantArrayBounds();
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
    TypeName tn { TypeSpecifier { std::make_shared<IdentifierExpression>("x", translation_unit::Context { "t", 1 }) },
            unnamedPointerDeclarator() };
    TypeSpecifier ts { std::move(tn) };
    ASSERT_TRUE(ts.resolveTypeofAtParseTime(env));
    EXPECT_TRUE(ts.getType().isPointer());
    EXPECT_TRUE(ts.getType().dereference().equivalentTo(type::signedInteger()));
    EXPECT_FALSE(ts.needsSemanticResolve());

    TypeSpecifier viaDeclarator { std::make_shared<IdentifierExpression>("x", translation_unit::Context { "t", 1 }) };
    viaDeclarator.deferAbstractDeclarator(unnamedPointerDeclarator());
    ASSERT_TRUE(viaDeclarator.resolveTypeofAtParseTime(env));
    EXPECT_TRUE(viaDeclarator.getType().isPointer());
    EXPECT_TRUE(viaDeclarator.getType().dereference().equivalentTo(type::signedInteger()));

    TypeSpecifier unknown { std::make_shared<IdentifierExpression>("nope", translation_unit::Context { "t", 1 }) };
    EXPECT_FALSE(unknown.resolveTypeofAtParseTime(env));
    EXPECT_TRUE(unknown.needsSemanticResolve());
}

TEST(FormalArgument, pendingTypeofIsNotVoidParameter) {
    TypeSpecifier ts { std::make_shared<IdentifierExpression>("x", translation_unit::Context { "t", 1 }) };
    FormalArgument arg { DeclarationSpecifiers { ts } };
    EXPECT_TRUE(arg.needsSemanticResolve());
    EXPECT_FALSE(arg.isVoid());
}

} // namespace
