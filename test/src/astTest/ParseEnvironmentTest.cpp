#include "gtest/gtest.h"

#include "ast/ParseEnvironment.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"

using namespace ast;
using namespace scanner;

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
    auto ended = env.endEnumDefinition();
    ASSERT_EQ(ended.size(), 3u);
    EXPECT_EQ(session.enums.entries().at("GREEN"), 10);
}
