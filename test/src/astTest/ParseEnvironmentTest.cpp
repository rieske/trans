#include "gtest/gtest.h"

#include <memory>
#include <stdexcept>
#include <vector>

#include "ast/ArrayDeclarator.h"
#include "ast/Constant.h"
#include "ast/ConstantExpression.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/FormalArgument.h"
#include "ast/Identifier.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializedDeclarator.h"
#include "ast/Operator.h"
#include "ast/ParseEnvironment.h"
#include "ast/PostfixExpression.h"
#include "ast/PrefixExpression.h"
#include "ast/StorageSpecifier.h"
#include "ast/TerminalSymbol.h"
#include "ast/TypeSpecifier.h"
#include "ast/UnaryExpression.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"

using namespace ast;
using namespace scanner;

namespace {

std::unique_ptr<Declarator> namedDeclarator(const std::string& name) {
    TerminalSymbol id { "id", name, { "t", 1 } };
    return std::make_unique<Declarator>(std::make_unique<Identifier>(id));
}

std::unique_ptr<InitializedDeclarator> plainDeclarator(const std::string& name) {
    return std::make_unique<InitializedDeclarator>(namedDeclarator(name));
}

} // namespace

TEST(ParseEnvironment, ensureStructTagSharesIdentity) {
    LexicalSession session;
    ParseEnvironment env{session};
    type::Type a = env.ensureStructTag("Node");
    type::Type b = env.ensureStructTag("Node");
    EXPECT_EQ(a.structureBodyIdentity(), b.structureBodyIdentity());
    type::Type c = env.ensureStructTag("Node");
    EXPECT_EQ(c.structureBodyIdentity(), a.structureBodyIdentity());
}

TEST(ParseEnvironment, nestedEnsureStructTagFindsParentTag) {
    LexicalSession session;
    ParseEnvironment parent{session};
    type::Type outer = parent.ensureStructTag("Pair");
    type::completeStructure(outer, { { "a", type::signedLong() }, { "b", type::signedLong() } });

    ParseEnvironment nested{session, parent};
    type::Type inner = nested.ensureStructTag("Pair");
    EXPECT_EQ(inner.structureBodyIdentity(), outer.structureBodyIdentity());
    EXPECT_TRUE(inner.isCompleteStructure());
    EXPECT_EQ(inner.getSize(), 16u);

    type::Type local = nested.ensureStructTag("Inner");
    EXPECT_TRUE(local.isIncompleteStructure());
    EXPECT_NE(local.structureBodyIdentity(), outer.structureBodyIdentity());
    type::Type parentInner = parent.ensureStructTag("Inner");
    EXPECT_NE(parentInner.structureBodyIdentity(), local.structureBodyIdentity());
}

TEST(ParseEnvironment, typedefAndEnumThroughSession) {
    LexicalSession session;
    ParseEnvironment env{session};
    env.defineTypedef("myint", type::signedInteger());
    auto t = env.lookupTypedef("myint");
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(session.typedefs.has("myint"));

    env.addEnumerator("RED");
    env.addEnumerator("GREEN", 10);
    env.addEnumerator("BLUE");
    long v = 0;
    EXPECT_TRUE(env.lookupEnumConstant("RED", v));
    EXPECT_EQ(v, 0);
    EXPECT_TRUE(env.lookupEnumConstant("GREEN", v));
    EXPECT_EQ(v, 10);
    EXPECT_TRUE(env.lookupEnumConstant("BLUE", v));
    EXPECT_EQ(v, 11);
    env.endEnumDefinition();
    EXPECT_EQ(session.enums.entries().at("GREEN"), 10);
    EXPECT_EQ(session.enums.entries().size(), 3u);
}

TEST(ParseEnvironment, enumeratorRedefinitionThrows) {
    LexicalSession session;
    ParseEnvironment env{session};
    env.addEnumerator("A", 1);
    EXPECT_THROW(env.addEnumerator("A", 1), std::runtime_error);
    EXPECT_THROW(env.addEnumerator("A", 2), std::runtime_error);
}

TEST(ParseEnvironment, registerInitializedDeclarationDefinesTypedef) {
    LexicalSession session;
    ParseEnvironment env{session};
    translation_unit::Context ctx { "t", 1 };
    DeclarationSpecifiers specs {
            StorageSpecifier::TYPEDEF(ctx),
            DeclarationSpecifiers { TypeSpecifier { type::signedInteger(), "int" } } };
    std::vector<std::unique_ptr<InitializedDeclarator>> decls;
    decls.push_back(plainDeclarator("myint"));
    env.registerInitializedDeclaration(specs, decls);
    auto t = env.lookupTypedef("myint");
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(session.typedefs.has("myint"));
}

TEST(ParseEnvironment, registerInitializedDeclarationEmptyTypedefSpecsNoAlias) {
    // Incomplete reduction: typedef storage with no type-specs is a soft no-op.
    LexicalSession session;
    ParseEnvironment env{session};
    translation_unit::Context ctx { "t", 1 };
    DeclarationSpecifiers specs { StorageSpecifier::TYPEDEF(ctx) };
    std::vector<std::unique_ptr<InitializedDeclarator>> decls;
    decls.push_back(plainDeclarator("myint"));
    env.registerInitializedDeclaration(specs, decls);
    EXPECT_FALSE(env.lookupTypedef("myint").has_value());
    EXPECT_FALSE(session.typedefs.has("myint"));
}

TEST(ParseEnvironment, registerInitializedDeclarationShadowsObjectReuse) {
    LexicalSession session;
    ParseEnvironment env{session};
    env.defineTypedef("T", type::signedInteger());
    DeclarationSpecifiers specs { TypeSpecifier { type::signedInteger(), "int" } };
    std::vector<std::unique_ptr<InitializedDeclarator>> decls;
    decls.push_back(plainDeclarator("T"));
    env.registerInitializedDeclaration(specs, decls);
    EXPECT_TRUE(session.typedefs.isIdentifierShadow("T"));
}

TEST(ParseEnvironment, defineObjectIsBraceScoped) {
    LexicalSession session;
    ParseEnvironment env{session};
    env.defineObject("x", type::signedInteger());
    auto outer = env.lookupObject("x");
    ASSERT_TRUE(outer.has_value());
    EXPECT_TRUE(outer->equivalentTo(type::signedInteger()));

    session.enterBlock();
    env.defineObject("x", type::signedCharacter());
    auto inner = env.lookupObject("x");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    session.leaveBlock();

    auto restored = env.lookupObject("x");
    ASSERT_TRUE(restored.has_value());
    EXPECT_TRUE(restored->equivalentTo(type::signedInteger()));
}

TEST(ParseEnvironment, maybeRegisterParameterShadowPending) {
    LexicalSession session;
    ParseEnvironment env{session};
    env.defineTypedef("T", type::signedInteger());
    env.maybeRegisterParameterShadow("T");
    EXPECT_FALSE(session.typedefs.isIdentifierShadow("T"));
    session.typedefs.pushIdentifierShadowScope();
    session.typedefs.flushPendingParameterShadows();
    EXPECT_TRUE(session.typedefs.isIdentifierShadow("T"));
}

TEST(ParseEnvironment, maybeRegisterParameterShadowNoopsForEmptyOrUnknown) {
    LexicalSession session;
    ParseEnvironment env{session};
    env.defineTypedef("T", type::signedInteger());
    env.maybeRegisterParameterShadow("");
    env.maybeRegisterParameterShadow("not_a_typedef");
    session.typedefs.pushIdentifierShadowScope();
    session.typedefs.flushPendingParameterShadows();
    EXPECT_FALSE(session.typedefs.isIdentifierShadow("T"));
    EXPECT_FALSE(session.typedefs.isIdentifierShadow("not_a_typedef"));
}

TEST(ParseEnvironment, typeOfIdentifierEnumUnaryAndTyped) {
    LexicalSession session;
    ParseEnvironment env{session};
    translation_unit::Context ctx { "t", 1 };
    env.defineObject("x", type::signedInteger());
    env.defineObject("p", type::pointer(type::signedInteger()));
    env.defineObject("a", type::array(type::signedCharacter(), 4));
    env.addEnumerator("RED");

    IdentifierExpression id { "x", ctx };
    auto idType = env.typeOf(id);
    ASSERT_TRUE(idType.has_value());
    EXPECT_TRUE(idType->equivalentTo(type::signedInteger()));

    IdentifierExpression enumerator { "RED", ctx };
    auto enumType = env.typeOf(enumerator);
    ASSERT_TRUE(enumType.has_value());
    EXPECT_TRUE(enumType->equivalentTo(type::signedInteger()));

    IdentifierExpression unknown { "nope", ctx };
    EXPECT_FALSE(env.typeOf(unknown).has_value());

    ConstantExpression constant { Constant { "1", type::signedInteger(), ctx } };
    auto constantType = env.typeOf(constant);
    ASSERT_TRUE(constantType.has_value());
    EXPECT_TRUE(constantType->equivalentTo(type::signedInteger()));

    UnaryExpression deref {
            std::make_unique<Operator>("*"),
            std::make_unique<IdentifierExpression>("p", ctx) };
    auto derefType = env.typeOf(deref);
    ASSERT_TRUE(derefType.has_value());
    EXPECT_TRUE(derefType->equivalentTo(type::signedInteger()));

    UnaryExpression addr {
            std::make_unique<Operator>("&"),
            std::make_unique<IdentifierExpression>("x", ctx) };
    auto addrType = env.typeOf(addr);
    ASSERT_TRUE(addrType.has_value());
    EXPECT_TRUE(addrType->isPointer());
    EXPECT_TRUE(addrType->dereference().equivalentTo(type::signedInteger()));

    UnaryExpression arrayDeref {
            std::make_unique<Operator>("*"),
            std::make_unique<IdentifierExpression>("a", ctx) };
    auto elementType = env.typeOf(arrayDeref);
    ASSERT_TRUE(elementType.has_value());
    EXPECT_TRUE(elementType->equivalentTo(type::signedCharacter()));

    UnaryExpression plus {
            std::make_unique<Operator>("+"),
            std::make_unique<IdentifierExpression>("x", ctx) };
    EXPECT_FALSE(env.typeOf(plus).has_value());

    PrefixExpression prefix {
            std::make_unique<Operator>("++"),
            std::make_unique<IdentifierExpression>("x", ctx) };
    auto prefixType = env.typeOf(prefix);
    ASSERT_TRUE(prefixType.has_value());
    EXPECT_TRUE(prefixType->equivalentTo(type::signedInteger()));

    PostfixExpression postfix {
            std::make_unique<IdentifierExpression>("x", ctx),
            std::make_unique<Operator>("++") };
    auto postfixType = env.typeOf(postfix);
    ASSERT_TRUE(postfixType.has_value());
    EXPECT_TRUE(postfixType->equivalentTo(type::signedInteger()));
}

TEST(ParseEnvironment, parameterPendingIsVisibleThenCleared) {
    LexicalSession session;
    ParseEnvironment env{session};
    FormalArgument arg {
            DeclarationSpecifiers { TypeSpecifier { type::signedInteger(), "int" } },
            namedDeclarator("n") };
    env.maybeDefineParameter(arg);
    auto pending = env.typeOf(IdentifierExpression { "n", translation_unit::Context { "t", 1 } });
    ASSERT_TRUE(pending.has_value());
    EXPECT_TRUE(pending->equivalentTo(type::signedInteger()));

    session.endDeclarators();
    EXPECT_FALSE(env.lookupObject("n").has_value());
    EXPECT_FALSE(env.typeOf(IdentifierExpression { "n", translation_unit::Context { "t", 1 } }).has_value());
}

TEST(ParseEnvironment, parameterPendingFlushesOnEnterBlock) {
    LexicalSession session;
    ParseEnvironment env{session};
    FormalArgument arg {
            DeclarationSpecifiers { TypeSpecifier { type::signedInteger(), "int" } },
            namedDeclarator("n") };
    env.maybeDefineParameter(arg);
    session.enterBlock();
    auto body = env.lookupObject("n");
    ASSERT_TRUE(body.has_value());
    EXPECT_TRUE(body->equivalentTo(type::signedInteger()));
    session.leaveBlock();
    EXPECT_FALSE(env.lookupObject("n").has_value());
}

TEST(ParseEnvironment, parameterArrayDecaysToPointer) {
    LexicalSession session;
    ParseEnvironment env{session};
    translation_unit::Context ctx { "t", 1 };
    FormalArgument arg {
            DeclarationSpecifiers { TypeSpecifier { type::signedInteger(), "int" } },
            std::make_unique<Declarator>(std::make_unique<ArrayDeclarator>(
                    std::make_unique<Identifier>(TerminalSymbol { "id", "a", ctx }),
                    nullptr)) };
    EXPECT_TRUE(arg.getType().isPointer());
    env.maybeDefineParameter(arg);
    auto t = env.typeOf(IdentifierExpression { "a", ctx });
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->isPointer());
    EXPECT_TRUE(t->dereference().equivalentTo(type::signedInteger()));
}

TEST(ParseEnvironment, parameterIncompleteArrayIsSkipped) {
    LexicalSession session;
    ParseEnvironment env{session};
    translation_unit::Context ctx { "t", 1 };
    FormalArgument arg {
            DeclarationSpecifiers { TypeSpecifier { type::voidType(), "void" } },
            std::make_unique<Declarator>(std::make_unique<ArrayDeclarator>(
                    std::make_unique<Identifier>(TerminalSymbol { "id", "a", ctx }),
                    std::make_unique<ConstantExpression>(
                            Constant { "3", type::signedInteger(), ctx }))) };
    EXPECT_THROW(arg.getType(), std::invalid_argument);
    EXPECT_NO_THROW(env.maybeDefineParameter(arg));
    EXPECT_FALSE(env.lookupObject("a").has_value());
}

TEST(ParseEnvironment, registerInitializedDeclarationSkipsPendingTypeof) {
    LexicalSession session;
    ParseEnvironment env{session};
    DeclarationSpecifiers specs { TypeSpecifier {
            std::make_shared<IdentifierExpression>("nope", translation_unit::Context { "t", 1 }) } };
    ASSERT_TRUE(specs.needsSemanticResolve());
    std::vector<std::unique_ptr<InitializedDeclarator>> decls;
    decls.push_back(plainDeclarator("y"));
    env.registerInitializedDeclaration(specs, decls);
    EXPECT_FALSE(env.lookupObject("y").has_value());
}
