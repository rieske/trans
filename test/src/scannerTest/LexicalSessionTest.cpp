#include "gtest/gtest.h"

#include "scanner/LexicalSession.h"
#include "types/IntegerConstant.h"
#include "types/Type.h"

using namespace scanner;

TEST(LexicalSession, isTypedefDelegatesToRegistry) {
    LexicalSession session;
    EXPECT_FALSE(session.isTypedef("T"));
    session.names.addTypedef("T", type::signedInteger());
    EXPECT_TRUE(session.isTypedef("T"));
    EXPECT_TRUE(session.names.hasTypedef("T"));
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

    EXPECT_TRUE(session.names.lookupTypedef("_Float32")->equivalentTo(type::floating()));
    EXPECT_TRUE(session.names.lookupTypedef("_Float64")->equivalentTo(type::doubleFloating()));
    EXPECT_TRUE(session.names.lookupTypedef("_Float128")->equivalentTo(type::doubleFloating()));
    EXPECT_TRUE(session.names.lookupTypedef("_Float32x")->equivalentTo(type::floating()));
    EXPECT_TRUE(session.names.lookupTypedef("_Float64x")->equivalentTo(type::doubleFloating()));
}

TEST(LexicalSession, typedefAndEnumAreInstanceOwned) {
    LexicalSession a;
    LexicalSession b;
    a.names.addTypedef("T", type::signedInteger());
    a.enums.add("E", type::fromHostLong(3));
    EXPECT_TRUE(a.names.hasTypedef("T"));
    EXPECT_FALSE(b.names.hasTypedef("T"));
    type::IntegerConstant v;
    EXPECT_TRUE(a.enums.lookup("E", v));
    EXPECT_EQ(type::toHostLong(v), 3);
    EXPECT_FALSE(b.enums.lookup("E", v));
}

TEST(IdentifierTable, shadowScopesPushPop) {
    IdentifierTable reg;
    reg.addTypedef("T", type::signedInteger());
    reg.enterScope();
    reg.addIdentifierShadow("T");
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    reg.leaveScope();
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
}

TEST(IdentifierTable, fileScopeAutoRootShadow) {
    IdentifierTable reg;
    reg.addTypedef("T", type::signedInteger());
    reg.addIdentifierShadow("T");
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
}

TEST(IdentifierTable, extraLeaveScopeKeepsFileScopeShadow) {
    IdentifierTable reg;
    reg.addTypedef("T", type::signedInteger());
    reg.addIdentifierShadow("T");
    reg.leaveScope();
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    EXPECT_TRUE(reg.hasTypedef("T"));
    reg.leaveScope();
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    EXPECT_TRUE(reg.hasTypedef("T"));
}

TEST(IdentifierTable, addTypedefClearsShadowOfSameName) {
    IdentifierTable reg;
    reg.enterScope();
    reg.addIdentifierShadow("T");
    reg.addTypedef("T", type::signedInteger());
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
    EXPECT_TRUE(reg.hasTypedef("T"));
}

TEST(IdentifierTable, addTypedefLastWins) {
    IdentifierTable reg;
    reg.addTypedef("T", type::signedInteger());
    reg.addTypedef("T", type::unsignedInteger());
    auto t = reg.lookupTypedef("T");
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

TEST(IdentifierTable, pendingParameterShadowFlushesOnScope) {
    IdentifierTable reg;
    reg.addTypedef("T", type::signedInteger());
    reg.addPendingParameterShadow("T");
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
    reg.enterScope();
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    reg.leaveScope();
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
}

TEST(IdentifierTable, pendingParameterShadowClearedWithoutFlush) {
    IdentifierTable reg;
    reg.addTypedef("T", type::signedInteger());
    reg.addPendingParameterShadow("T");
    reg.clearPendingParameterShadows();
    reg.enterScope();
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
}

TEST(IdentifierTable, objectAddLookupAndLastWins) {
    IdentifierTable names;
    names.addObject("x", type::signedInteger());
    auto t = names.lookupObject("x");
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->equivalentTo(type::signedInteger()));
    names.addObject("x", type::unsignedInteger());
    t = names.lookupObject("x");
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->equivalentTo(type::unsignedInteger()));
    EXPECT_FALSE(names.lookupObject("missing").has_value());
}

TEST(IdentifierTable, pendingObjectVisibleThenCleared) {
    IdentifierTable names;
    names.addPendingObject("n", type::signedInteger());
    auto pending = names.lookupObject("n");
    ASSERT_TRUE(pending.has_value());
    EXPECT_TRUE(pending->equivalentTo(type::signedInteger()));
    names.clearPendingObjects();
    EXPECT_FALSE(names.lookupObject("n").has_value());
}

TEST(IdentifierTable, pendingObjectFlushesIntoScope) {
    IdentifierTable names;
    names.addPendingObject("n", type::signedInteger());
    names.enterScope();
    auto t = names.lookupObject("n");
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->equivalentTo(type::signedInteger()));
    names.leaveScope();
    EXPECT_FALSE(names.lookupObject("n").has_value());
}

TEST(IdentifierTable, objectBraceScopesAndExtraLeaveKeepsRoot) {
    IdentifierTable names;
    names.addObject("x", type::signedInteger());
    names.enterScope();
    names.addObject("x", type::signedCharacter());
    auto inner = names.lookupObject("x");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    names.leaveScope();
    auto outer = names.lookupObject("x");
    ASSERT_TRUE(outer.has_value());
    EXPECT_TRUE(outer->equivalentTo(type::signedInteger()));
    names.leaveScope();
    EXPECT_TRUE(names.lookupObject("x").has_value());
    names.leaveScope();
    names.addObject("y", type::signedInteger());
    auto y = names.lookupObject("y");
    ASSERT_TRUE(y.has_value());
    EXPECT_TRUE(y->equivalentTo(type::signedInteger()));
}

TEST(IdentifierTable, oneEnterScopeHopsTypedefsAndObjects) {
    IdentifierTable names;
    names.addTypedef("T", type::signedInteger());
    names.addObject("x", type::signedInteger());
    names.enterScope();
    names.addTypedef("T", type::signedCharacter());
    names.addObject("x", type::signedCharacter());
    auto innerTypedef = names.lookupTypedef("T");
    auto innerObject = names.lookupObject("x");
    ASSERT_TRUE(innerTypedef.has_value());
    ASSERT_TRUE(innerObject.has_value());
    EXPECT_TRUE(innerTypedef->equivalentTo(type::signedCharacter()));
    EXPECT_TRUE(innerObject->equivalentTo(type::signedCharacter()));
    names.leaveScope();
    auto outerTypedef = names.lookupTypedef("T");
    auto outerObject = names.lookupObject("x");
    ASSERT_TRUE(outerTypedef.has_value());
    ASSERT_TRUE(outerObject.has_value());
    EXPECT_TRUE(outerTypedef->equivalentTo(type::signedInteger()));
    EXPECT_TRUE(outerObject->equivalentTo(type::signedInteger()));
}

TEST(LexicalSession, endDeclaratorsClearsPendingParameterShadows) {
    LexicalSession session;
    session.names.addTypedef("T", type::signedInteger());
    session.names.addPendingParameterShadow("T");
    session.endDeclarators();
    session.enterBlock();
    EXPECT_FALSE(session.names.isIdentifierShadow("T"));
}

TEST(LexicalSession, enterBlockSharesShadowAndObjectFrames) {
    LexicalSession session;
    session.names.addTypedef("T", type::signedInteger());
    session.names.addObject("x", type::signedInteger());
    session.enums.add("E", type::fromHostLong(1));
    session.enterBlock();
    session.names.addIdentifierShadow("T");
    session.names.addObject("x", type::signedCharacter());
    EXPECT_TRUE(session.names.isIdentifierShadow("T"));
    auto inner = session.names.lookupObject("x");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    session.leaveBlock();
    EXPECT_FALSE(session.names.isIdentifierShadow("T"));
    EXPECT_TRUE(session.names.hasTypedef("T"));
    EXPECT_TRUE(session.isTypedef("T"));
    EXPECT_TRUE(session.isEnumerator("E"));
    auto outer = session.names.lookupObject("x");
    ASSERT_TRUE(outer.has_value());
    EXPECT_TRUE(outer->equivalentTo(type::signedInteger()));
}

TEST(LexicalSession, blockTypedefIsGoneAfterLeaveBlock) {
    LexicalSession session;
    session.enterBlock();
    session.names.addTypedef("T", type::signedInteger());
    EXPECT_TRUE(session.isTypedef("T"));
    session.leaveBlock();
    EXPECT_FALSE(session.isTypedef("T"));
    EXPECT_TRUE(session.isTypedef("_Float32"));
}

TEST(LexicalSession, leaveBlockRestoresOuterTypedef) {
    LexicalSession session;
    session.names.addTypedef("T", type::signedInteger());
    session.enterBlock();
    session.names.addTypedef("T", type::signedCharacter());
    auto inner = session.names.lookupTypedef("T");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    session.leaveBlock();
    auto outer = session.names.lookupTypedef("T");
    ASSERT_TRUE(outer.has_value());
    EXPECT_TRUE(outer->equivalentTo(type::signedInteger()));
}

TEST(IdentifierTable, leaveScopeRemovesInnerTypedefOnly) {
    IdentifierTable reg;
    reg.addTypedef("T", type::signedInteger());
    reg.enterScope();
    reg.addTypedef("U", type::unsignedInteger());
    EXPECT_TRUE(reg.hasTypedef("T"));
    EXPECT_TRUE(reg.hasTypedef("U"));
    reg.leaveScope();
    EXPECT_TRUE(reg.hasTypedef("T"));
    EXPECT_FALSE(reg.hasTypedef("U"));
}

TEST(IdentifierTable, extraLeaveScopeKeepsRootTypedef) {
    IdentifierTable reg;
    reg.addTypedef("T", type::signedInteger());
    reg.leaveScope();
    EXPECT_TRUE(reg.hasTypedef("T"));
    reg.leaveScope();
    EXPECT_TRUE(reg.hasTypedef("T"));
}

TEST(IdentifierTable, innerTypedefDoesNotClearOuterObjectShadow) {
    IdentifierTable reg;
    reg.addTypedef("T", type::signedInteger());
    reg.enterScope();
    reg.addIdentifierShadow("T");
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    reg.enterScope();
    reg.addTypedef("T", type::signedCharacter());
    EXPECT_FALSE(reg.isIdentifierShadow("T"));
    auto inner = reg.lookupTypedef("T");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    reg.leaveScope();
    EXPECT_TRUE(reg.isIdentifierShadow("T"));
    auto outer = reg.lookupTypedef("T");
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
    session.names.addTypedef("T", type::signedInteger());
    session.names.addIdentifierShadow("T");
    session.names.addObject("x", type::signedInteger());
    session.leaveBlock();
    EXPECT_TRUE(session.isTypedef("T"));
    EXPECT_TRUE(session.names.isIdentifierShadow("T"));
    EXPECT_TRUE(session.isTypedef("_Float32"));
    auto x = session.names.lookupObject("x");
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
    session.names.addObject("x", type::signedInteger());
    session.enums.add("A", type::fromHostLong(1));
    session.openBrace(BraceFrame::Record);
    session.names.addObject("x", type::signedCharacter());
    session.enums.add("B", type::fromHostLong(2));
    auto inner = session.names.lookupObject("x");
    ASSERT_TRUE(inner.has_value());
    EXPECT_TRUE(inner->equivalentTo(type::signedCharacter()));
    session.closeBrace();
    auto outer = session.names.lookupObject("x");
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
