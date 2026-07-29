#include "gtest/gtest.h"

#include <memory>
#include <stdexcept>
#include <vector>

#include "ast/DeclarationSpecifiers.h"
#include "ast/Identifier.h"
#include "ast/InitializedDeclarator.h"
#include "ast/ParseEnvironment.h"
#include "ast/StorageSpecifier.h"
#include "ast/TerminalSymbol.h"
#include "ast/TypeSpecifier.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"

using namespace ast;
using namespace scanner;

namespace {

std::unique_ptr<InitializedDeclarator> plainDeclarator(const std::string& name) {
    TerminalSymbol id { "id", name, { "t", 1 } };
    return std::make_unique<InitializedDeclarator>(
            std::make_unique<Declarator>(std::make_unique<Identifier>(id)));
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
