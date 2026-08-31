#include "gtest/gtest.h"

#include "scanner/LexicalSession.h"
#include "types/IntegerConstant.h"
#include "types/Type.h"

using namespace scanner;

TEST(LexicalSession, isTypedefDelegatesToRegistry) {
    LexicalSession session;
    EXPECT_FALSE(session.isTypedef("T"));
    session.typedefs.add("T", type::signedInteger());
    EXPECT_TRUE(session.isTypedef("T"));
    EXPECT_TRUE(session.typedefs.has("T"));
}

TEST(LexicalSession, isEnumeratorDelegatesToRegistry) {
    LexicalSession session;
    EXPECT_FALSE(session.isEnumerator("E"));
    session.enums.add("E", type::fromHostLong(3));
    EXPECT_TRUE(session.isEnumerator("E"));
    EXPECT_TRUE(session.enums.contains("E"));
    type::IntegerConstant v;
    ASSERT_TRUE(session.lookupEnumerator("E", v));
    EXPECT_EQ(type::toHostLong(v), 3);
    EXPECT_FALSE(session.lookupEnumerator("missing", v));
}

TEST(LexicalSession, defaultSessionHasBuiltinFloatTypedefs) {
    LexicalSession session;

    EXPECT_TRUE(session.isTypedef("_Float32"));
    EXPECT_TRUE(session.isTypedef("_Float64"));
    EXPECT_TRUE(session.isTypedef("_Float128"));
    EXPECT_TRUE(session.isTypedef("_Float32x"));
    EXPECT_TRUE(session.isTypedef("_Float64x"));

    EXPECT_TRUE(session.typedefs.tryLookup("_Float32")->equivalentTo(type::floating()));
    EXPECT_TRUE(session.typedefs.tryLookup("_Float64")->equivalentTo(type::doubleFloating()));
    EXPECT_TRUE(session.typedefs.tryLookup("_Float128")->equivalentTo(type::doubleFloating()));
    EXPECT_TRUE(session.typedefs.tryLookup("_Float32x")->equivalentTo(type::floating()));
    EXPECT_TRUE(session.typedefs.tryLookup("_Float64x")->equivalentTo(type::doubleFloating()));
}

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

TEST(TypedefRegistry, shadowScopesPushPop) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.enterScope();
    reg.addIdentifierShadow("T");
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    reg.leaveScope();
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
}

TEST(TypedefRegistry, fileScopeAutoRootShadow) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.addIdentifierShadow("T");
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
}

TEST(TypedefRegistry, extraLeaveScopeKeepsFileScopeShadow) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.addIdentifierShadow("T");
    reg.leaveScope();
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    EXPECT_TRUE(reg.has("T"));
    reg.leaveScope();
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    EXPECT_TRUE(reg.has("T"));
}

TEST(TypedefRegistry, addClearsShadowOfSameName) {
    TypedefRegistry reg;
    reg.enterScope();
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

TEST(EnumConstantRegistry, leaveScopeRemovesInnerOnly) {
    EnumConstantRegistry enums;
    enums.add("A", type::fromHostLong(1));
    enums.enterScope();
    enums.add("B", type::fromHostLong(2));
    EXPECT_TRUE(enums.contains("A"));
    EXPECT_TRUE(enums.contains("B"));
    enums.leaveScope();
    EXPECT_TRUE(enums.contains("A"));
    EXPECT_FALSE(enums.contains("B"));
}

TEST(EnumConstantRegistry, extraLeaveScopeKeepsRoot) {
    EnumConstantRegistry enums;
    enums.add("A", type::fromHostLong(1));
    enums.leaveScope();
    EXPECT_TRUE(enums.contains("A"));
    enums.leaveScope();
    EXPECT_TRUE(enums.contains("A"));
}

TEST(EnumConstantRegistry, innerHidesOuterAndRestores) {
    EnumConstantRegistry enums;
    enums.add("A", type::fromHostLong(1));
    enums.enterScope();
    enums.add("A", type::fromHostLong(2));
    type::IntegerConstant v;
    ASSERT_TRUE(enums.lookup("A", v));
    EXPECT_EQ(type::toHostLong(v), 2);
    EXPECT_TRUE(enums.containsInCurrentScope("A"));
    enums.leaveScope();
    ASSERT_TRUE(enums.lookup("A", v));
    EXPECT_EQ(type::toHostLong(v), 1);
}

TEST(TypedefRegistry, pendingParameterShadowFlushesOnScope) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.addPendingParameterShadow("T");
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
    reg.enterScope();
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    reg.leaveScope();
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
}

TEST(TypedefRegistry, pendingParameterShadowClearedWithoutFlush) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.addPendingParameterShadow("T");
    reg.clearPendingParameterShadows();
    reg.enterScope();
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
    objects.enterScope();
    auto t = objects.lookup("n");
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->equivalentTo(type::signedInteger()));
    objects.leaveScope();
    EXPECT_FALSE(objects.lookup("n").has_value());
}

TEST(ObjectTypeRegistry, braceScopesAndExtraLeaveKeepsRoot) {
    ObjectTypeRegistry objects;
    objects.add("x", type::signedInteger());
    objects.enterScope();
    objects.add("x", type::signedCharacter());
    auto inner = objects.lookup("x");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    objects.leaveScope();
    auto outer = objects.lookup("x");
    ASSERT_TRUE(outer.has_value());
    EXPECT_TRUE(outer->equivalentTo(type::signedInteger()));
    objects.leaveScope();
    EXPECT_TRUE(objects.lookup("x").has_value());
    objects.leaveScope();
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
    session.enums.add("E", type::fromHostLong(1));
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
    EXPECT_TRUE(session.isTypedef("T"));
    EXPECT_TRUE(session.isEnumerator("E"));
    auto outer = session.objects.lookup("x");
    ASSERT_TRUE(outer.has_value());
    EXPECT_TRUE(outer->equivalentTo(type::signedInteger()));
}

TEST(LexicalSession, blockTypedefIsGoneAfterLeaveBlock) {
    LexicalSession session;
    session.enterBlock();
    session.typedefs.add("T", type::signedInteger());
    EXPECT_TRUE(session.isTypedef("T"));
    session.leaveBlock();
    EXPECT_FALSE(session.isTypedef("T"));
    EXPECT_TRUE(session.isTypedef("_Float32"));
}

TEST(LexicalSession, leaveBlockRestoresOuterTypedef) {
    LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    session.enterBlock();
    session.typedefs.add("T", type::signedCharacter());
    auto inner = session.typedefs.tryLookup("T");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    session.leaveBlock();
    auto outer = session.typedefs.tryLookup("T");
    ASSERT_TRUE(outer.has_value());
    EXPECT_TRUE(outer->equivalentTo(type::signedInteger()));
}

TEST(TypedefRegistry, leaveScopeRemovesInnerTypedefOnly) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.enterScope();
    reg.add("U", type::unsignedInteger());
    EXPECT_TRUE(reg.has("T"));
    EXPECT_TRUE(reg.has("U"));
    reg.leaveScope();
    EXPECT_TRUE(reg.has("T"));
    EXPECT_FALSE(reg.has("U"));
}

TEST(TypedefRegistry, extraLeaveScopeKeepsRootBinding) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.leaveScope();
    EXPECT_TRUE(reg.has("T"));
    reg.leaveScope();
    EXPECT_TRUE(reg.has("T"));
}

TEST(TypedefRegistry, innerTypedefDoesNotClearOuterObjectShadow) {
    TypedefRegistry reg;
    reg.add("T", type::signedInteger());
    reg.enterScope();
    reg.addIdentifierShadow("T");
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    reg.enterScope();
    reg.add("T", type::signedCharacter());
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
    auto inner = reg.tryLookup("T");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    reg.leaveScope();
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    auto outer = reg.tryLookup("T");
    ASSERT_TRUE(outer.has_value());
    EXPECT_TRUE(outer->equivalentTo(type::signedInteger()));
}

TEST(LexicalSession, blockEnumeratorIsGoneAfterLeaveBlock) {
    LexicalSession session;
    session.enterBlock();
    session.enums.add("A", type::fromHostLong(7));
    EXPECT_TRUE(session.isEnumerator("A"));
    session.leaveBlock();
    EXPECT_FALSE(session.isEnumerator("A"));
}

TEST(LexicalSession, leaveBlockRestoresOuterEnumerator) {
    LexicalSession session;
    session.enums.add("A", type::fromHostLong(1));
    session.enterBlock();
    session.enums.add("A", type::fromHostLong(2));
    type::IntegerConstant v;
    ASSERT_TRUE(session.lookupEnumerator("A", v));
    EXPECT_EQ(type::toHostLong(v), 2);
    session.leaveBlock();
    ASSERT_TRUE(session.lookupEnumerator("A", v));
    EXPECT_EQ(type::toHostLong(v), 1);
}

TEST(LexicalSession, extraLeaveBlockKeepsFileScopeBindings) {
    LexicalSession session;
    session.typedefs.add("T", type::signedInteger());
    session.typedefs.addIdentifierShadow("T");
    session.objects.add("x", type::signedInteger());
    session.leaveBlock();
    EXPECT_TRUE(session.isTypedef("T"));
    EXPECT_TRUE(session.typedefs.isIdentifierShadow("T"));
    EXPECT_TRUE(session.isTypedef("_Float32"));
    auto x = session.objects.lookup("x");
    ASSERT_TRUE(x.has_value());
    EXPECT_TRUE(x->equivalentTo(type::signedInteger()));
}

TEST(LexicalSession, extraLeaveBlockKeepsFileScopeEnumerator) {
    LexicalSession session;
    session.enums.add("A", type::fromHostLong(1));
    session.leaveBlock();
    EXPECT_TRUE(session.isEnumerator("A"));
}

TEST(LexicalSession, enumBodyBraceDoesNotHopEnumeratorScope) {
    LexicalSession session;
    session.openBrace(BraceFrame::Block);
    session.openBrace(BraceFrame::EnumBody);
    session.enums.add("A", type::fromHostLong(7));
    session.closeBrace();
    EXPECT_TRUE(session.isEnumerator("A"));
    session.closeBrace();
    EXPECT_FALSE(session.isEnumerator("A"));
}

TEST(LexicalSession, recordBraceIsolatesObjectsNotEnumerators) {
    LexicalSession session;
    session.objects.add("x", type::signedInteger());
    session.enums.add("A", type::fromHostLong(1));
    session.openBrace(BraceFrame::Record);
    session.objects.add("x", type::signedCharacter());
    session.enums.add("B", type::fromHostLong(2));
    auto inner = session.objects.lookup("x");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    session.closeBrace();
    auto outer = session.objects.lookup("x");
    ASSERT_TRUE(outer.has_value());
    EXPECT_TRUE(outer->equivalentTo(type::signedInteger()));
    EXPECT_TRUE(session.isEnumerator("A"));
    EXPECT_TRUE(session.isEnumerator("B"));
}

TEST(LexicalSession, extraCloseBraceKeepsRoot) {
    LexicalSession session;
    session.enums.add("A", type::fromHostLong(1));
    session.closeBrace();
    EXPECT_TRUE(session.isEnumerator("A"));
}
