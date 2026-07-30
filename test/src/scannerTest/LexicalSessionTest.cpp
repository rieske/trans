#include "gtest/gtest.h"

#include "scanner/LexicalSession.h"
#include "types/Type.h"

using namespace scanner;

TEST(LexicalSession, typedefAndEnumAreInstanceOwned) {
    LexicalSession a;
    LexicalSession b;
    a.typedefs.add("T", type::signedInteger());
    a.enums.add("E", 3);
    EXPECT_TRUE(a.typedefs.has("T"));
    EXPECT_FALSE(b.typedefs.has("T"));
    long v = 0;
    EXPECT_TRUE(a.enums.lookup("E", v));
    EXPECT_EQ(v, 3);
    EXPECT_FALSE(b.enums.lookup("E", v));
}

TEST(TypedefRegistry, shadowScopesPushPop) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.pushIdentifierShadowScope();
    reg.addIdentifierShadow("T");
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    reg.popIdentifierShadowScope();
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
}

TEST(TypedefRegistry, fileScopeAutoRootShadow) {
    // No prior push: add opens a root frame so file-scope object shadows work.
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.addIdentifierShadow("T");
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
}

TEST(TypedefRegistry, emptyPopIsNoOp) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.addIdentifierShadow("T");
    reg.popIdentifierShadowScope(); // root
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
    reg.popIdentifierShadowScope(); // extra empty pop: no throw, no erase
    reg.pushIdentifierShadowScope();
    reg.addIdentifierShadow("T");
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
}

TEST(TypedefRegistry, addClearsShadowOfSameName) {
    TypedefRegistry reg;
    reg.pushIdentifierShadowScope();
    reg.addIdentifierShadow("T");
    reg.add("T", type::signedInteger());
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
    EXPECT_TRUE(reg.has("T"));
}

TEST(TypedefRegistry, addLastWins) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.add("T", type::unsignedInteger());
    auto t = reg.tryLookup("T");
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->equivalentTo(type::unsignedInteger()));
    EXPECT_FALSE(t->equivalentTo(type::signedInteger()));
}

TEST(EnumConstantRegistry, addAndLookup) {
    EnumConstantRegistry enums;
    enums.add("A", 1);
    enums.add("B", 2);
    long v = 0;
    EXPECT_TRUE(enums.lookup("A", v));
    EXPECT_EQ(v, 1);
    EXPECT_TRUE(enums.lookup("B", v));
    EXPECT_EQ(v, 2);
    EXPECT_FALSE(enums.lookup("C", v));
}

TEST(EnumConstantRegistry, addLastWins) {
    EnumConstantRegistry enums;
    enums.add("A", 1);
    enums.add("A", 9);
    long v = 0;
    ASSERT_TRUE(enums.lookup("A", v));
    EXPECT_EQ(v, 9);
}
