#include "gtest/gtest.h"

#include "ast/DeclarationSpecifiers.h"
#include "ast/Declarator.h"
#include "ast/FormalArgument.h"
#include "ast/Identifier.h"
#include "ast/IdentifierExpression.h"
#include "ast/ParseEnvironment.h"
#include "ast/Pointer.h"
#include "ast/TerminalSymbol.h"
#include "ast/TypeSpecifier.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"

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

TEST(FormalArgument, pendingTypeofIsNotVoidParameter) {
    TypeSpecifier ts { std::make_shared<IdentifierExpression>("x", translation_unit::Context { "t", 1 }) };
    FormalArgument arg { DeclarationSpecifiers { ts } };
    EXPECT_TRUE(arg.needsSemanticResolve());
    EXPECT_FALSE(arg.isVoid());
}

} // namespace
