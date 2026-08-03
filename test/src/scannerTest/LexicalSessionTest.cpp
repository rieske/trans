#include "gtest/gtest.h"

#include "scanner/LexicalSession.h"
#include "types/IntegerConstant.h"
#include "types/Type.h"

using namespace scanner;

TEST(LexicalSession, typedefAndEnumAreInstanceOwned) {
    LexicalSession a;
    LexicalSession b;
    a.typedefs.add("T", type::signedInteger());
    a.enums.add("E", type::fromHostLong(3));
    EXPECT_TRUE(a.typedefs.has("T"));
    EXPECT_FALSE(b.typedefs.has("T"));
    type::IntegerConstant v;
    EXPECT_TRUE(a.enums.lookup("E", v));
    EXPECT_EQ(type::toHostLong(v), 3);
    EXPECT_FALSE(b.enums.lookup("E", v));
}

TEST(LexicalSession, sessionsAreIndependent) {
    LexicalSession a;
    LexicalSession b;
    a.typedefs.add("T", type::signedInteger());
    b.typedefs.add("T", type::signedLong());
    EXPECT_TRUE(a.typedefs.has("T"));
    EXPECT_TRUE(b.typedefs.has("T"));
    auto aType = a.typedefs.tryLookup("T");
    auto bType = b.typedefs.tryLookup("T");
    ASSERT_TRUE(aType.has_value());
    ASSERT_TRUE(bType.has_value());
    EXPECT_EQ(aType->getSize(), type::signedInteger().getSize());
    EXPECT_EQ(bType->getSize(), type::signedLong().getSize());
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
    enums.add("A", type::fromHostLong(1));
    enums.add("B", type::fromHostLong(2));
    type::IntegerConstant v;
    EXPECT_TRUE(enums.lookup("A", v));
    EXPECT_EQ(type::toHostLong(v), 1);
    EXPECT_TRUE(enums.lookup("B", v));
    EXPECT_EQ(type::toHostLong(v), 2);
    EXPECT_FALSE(enums.lookup("C", v));
}

TEST(EnumConstantRegistry, addLastWins) {
    EnumConstantRegistry enums;
    enums.add("A", type::fromHostLong(1));
    enums.add("A", type::fromHostLong(9));
    type::IntegerConstant v;
    ASSERT_TRUE(enums.lookup("A", v));
    EXPECT_EQ(type::toHostLong(v), 9);
}

TEST(TypedefRegistry, pendingParameterShadowFlushesOnScope) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.addPendingParameterShadow("T");
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
    reg.pushIdentifierShadowScope();
    reg.flushPendingParameterShadows();
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    reg.popIdentifierShadowScope();
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
}

TEST(TypedefRegistry, pendingParameterShadowClearedWithoutFlush) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.addPendingParameterShadow("T");
    reg.clearPendingParameterShadows();
    reg.pushIdentifierShadowScope();
    reg.flushPendingParameterShadows();
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
}

TEST(ObjectTypeRegistry, addLookupAndLastWins) {
    ObjectTypeRegistry objects;
    objects.add("x", type::signedInteger());
    auto t = objects.lookup("x");
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->equivalentTo(type::signedInteger()));
    objects.add("x", type::unsignedInteger());
    t = objects.lookup("x");
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->equivalentTo(type::unsignedInteger()));
    EXPECT_FALSE(objects.lookup("missing").has_value());
}

TEST(ObjectTypeRegistry, pendingVisibleThenCleared) {
    ObjectTypeRegistry objects;
    objects.addPending("n", type::signedInteger());
    auto pending = objects.lookup("n");
    ASSERT_TRUE(pending.has_value());
    EXPECT_TRUE(pending->equivalentTo(type::signedInteger()));
    objects.clearPending();
    EXPECT_FALSE(objects.lookup("n").has_value());
}

TEST(ObjectTypeRegistry, pendingFlushesIntoScope) {
    ObjectTypeRegistry objects;
    objects.addPending("n", type::signedInteger());
    objects.pushScope();
    objects.flushPending();
    auto t = objects.lookup("n");
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->equivalentTo(type::signedInteger()));
    objects.popScope();
    EXPECT_FALSE(objects.lookup("n").has_value());
}

TEST(ObjectTypeRegistry, braceScopesAndEmptyPop) {
    ObjectTypeRegistry objects;
    objects.add("x", type::signedInteger());
    objects.pushScope();
    objects.add("x", type::signedCharacter());
    auto inner = objects.lookup("x");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    objects.popScope();
    auto outer = objects.lookup("x");
    ASSERT_TRUE(outer.has_value());
    EXPECT_TRUE(outer->equivalentTo(type::signedInteger()));
    objects.popScope();
    EXPECT_FALSE(objects.lookup("x").has_value());
    objects.popScope();
    objects.add("y", type::signedInteger());
    auto y = objects.lookup("y");
    ASSERT_TRUE(y.has_value());
    EXPECT_TRUE(y->equivalentTo(type::signedInteger()));
}

TEST(LexicalSession, endDeclaratorsClearsPendingParameterShadows) {
    LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    session.typedefs.addPendingParameterShadow("T");
    session.endDeclarators();
    session.enterBlock();
    EXPECT_FALSE(session.typedefs.isIdentifierShadow("T"));
}

TEST(LexicalSession, enterBlockSharesShadowAndObjectFrames) {
    LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    session.objects.add("x", type::signedInteger());
    session.enterBlock();
    session.typedefs.addIdentifierShadow("T");
    session.objects.add("x", type::signedCharacter());
    EXPECT_TRUE(session.typedefs.isIdentifierShadow("T"));
    auto inner = session.objects.lookup("x");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    session.leaveBlock();
    EXPECT_FALSE(session.typedefs.isIdentifierShadow("T"));
    EXPECT_TRUE(session.typedefs.has("T"));
    auto outer = session.objects.lookup("x");
    ASSERT_TRUE(outer.has_value());
    EXPECT_TRUE(outer->equivalentTo(type::signedInteger()));
}
