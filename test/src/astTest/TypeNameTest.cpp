#include "gtest/gtest.h"

#include "ast/Declarator.h"
#include "ast/Identifier.h"
#include "ast/IdentifierExpression.h"
#include "ast/ParseEnvironment.h"
#include "ast/Pointer.h"
#include "ast/TerminalSymbol.h"
#include "ast/TypeName.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"

#include <memory>

namespace {

using namespace ast;

std::unique_ptr<Declarator> unnamedPointerDeclarator() {
    TerminalSymbol id { "id", "", { "t", 1 } };
    std::vector<Pointer> stars;
    stars.emplace_back();
    return std::make_unique<Declarator>(std::make_unique<Identifier>(id), std::move(stars));
}

TEST(TypeName, tryResolveKnownSpecWithoutDad) {
    scanner::LexicalSession session;
    ParseEnvironment env { session };
    TypeName tn { TypeSpecifier { type::signedInteger(), "int" }, nullptr };
    auto t = tn.tryResolve(env);
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->equivalentTo(type::signedInteger()));
}

TEST(TypeName, tryResolveAppliesDadToKnownSpec) {
    scanner::LexicalSession session;
    ParseEnvironment env { session };
    TypeName tn { TypeSpecifier { type::signedInteger(), "int" }, unnamedPointerDeclarator() };
    auto t = tn.tryResolve(env);
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->isPointer());
    EXPECT_TRUE(t->dereference().equivalentTo(type::signedInteger()));
}

TEST(TypeName, tryResolveTypeofViaEnvironment) {
    scanner::LexicalSession session;
    ParseEnvironment env { session };
    translation_unit::Context ctx { "t", 1 };
    env.defineObject("x", type::signedInteger());
    TypeName tn {
            TypeSpecifier { std::make_shared<IdentifierExpression>("x", ctx) },
            unnamedPointerDeclarator() };
    auto t = tn.tryResolve(env);
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->isPointer());
    EXPECT_TRUE(t->dereference().equivalentTo(type::signedInteger()));
}

TEST(TypeName, applyDadConsumesDeclarator) {
    TypeName tn { TypeSpecifier { type::signedInteger(), "int" }, unnamedPointerDeclarator() };
    type::Type t = tn.applyDad(tn.spec.getType());
    EXPECT_TRUE(t.isPointer());
    EXPECT_TRUE(t.dereference().equivalentTo(type::signedInteger()));
    EXPECT_EQ(tn.dad, nullptr);
    EXPECT_TRUE(tn.spec.getType().isPointer());
}

TEST(TypeName, applyDadWithoutDeclaratorIsIdentity) {
    TypeName tn { TypeSpecifier { type::signedInteger(), "int" }, nullptr };
    type::Type t = tn.applyDad(tn.spec.getType());
    EXPECT_TRUE(t.equivalentTo(type::signedInteger()));
}

TEST(TypeName, tryResolveUnknownTypeofIsEmpty) {
    scanner::LexicalSession session;
    ParseEnvironment env { session };
    TypeName tn {
            TypeSpecifier {
                    std::make_shared<IdentifierExpression>("nope",
                            translation_unit::Context { "t", 1 }) },
            nullptr };
    EXPECT_FALSE(tn.tryResolve(env).has_value());
}

} // namespace
