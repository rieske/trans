#include "gtest/gtest.h"

#include <memory>
#include <stdexcept>
#include <vector>

#include "ast/ArithmeticExpression.h"
#include "ast/ArrayAccess.h"
#include "ast/ArrayDeclarator.h"
#include "ast/Constant.h"
#include "ast/ConstantExpression.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/FormalArgument.h"
#include "ast/FunctionDeclarator.h"
#include "ast/Identifier.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializedDeclarator.h"
#include "ast/MemberAccess.h"
#include "ast/ParseEnvironment.h"
#include "ast/PostfixExpression.h"
#include "ast/PrefixExpression.h"
#include "ast/StorageSpecifier.h"
#include "ast/StringLiteralExpression.h"
#include "ast/TerminalSymbol.h"
#include "ast/TypeSpecifier.h"
#include "ast/UnaryExpression.h"
#include "scanner/LexicalSession.h"
#include "types/IntegerConstant.h"
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

TEST(ParseEnvironment, nestedEnvironmentSharesVlaExpressionTable) {
    LexicalSession session;
    ParseEnvironment parent{session};
    ParseEnvironment nested = ParseEnvironment::nestedIn(parent);
    EXPECT_EQ(&parent.vlaExpressions(), &nested.vlaExpressions());
}

TEST(ParseEnvironment, gnuExtensionsDefaultTrueAndCopiedToNested) {
    LexicalSession session;
    ParseEnvironment parent{session};
    EXPECT_TRUE(parent.gnuExtensions());
    parent.setGnuExtensions(false);
    EXPECT_FALSE(parent.gnuExtensions());
    ParseEnvironment nested{session, parent};
    EXPECT_FALSE(nested.gnuExtensions());
}

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
    type::completeStructure(outer, {
            type::MemberSpec { "a", type::signedLong() },
            type::MemberSpec { "b", type::signedLong() },
    });

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
    EXPECT_TRUE(session.names.hasTypedef("myint"));

    env.addEnumerator("RED");
    env.addEnumerator("GREEN", type::fromHostLong(10));
    env.addEnumerator("BLUE");
    type::IntegerConstant v;
    EXPECT_TRUE(env.lookupEnumConstant("RED", v));
    EXPECT_EQ(type::toHostLong(v), 0);
    EXPECT_TRUE(env.lookupEnumConstant("GREEN", v));
    EXPECT_EQ(type::toHostLong(v), 10);
    EXPECT_TRUE(env.lookupEnumConstant("BLUE", v));
    EXPECT_EQ(type::toHostLong(v), 11);
    auto underlying = env.endEnumDefinition("Color");
    EXPECT_TRUE(underlying.equivalentTo(type::signedInteger()));
    type::IntegerConstant stored;
    ASSERT_TRUE(session.enums.lookup("GREEN", stored));
    EXPECT_EQ(type::toHostLong(stored), 10);
    auto tag = env.lookupEnumTag("Color");
    ASSERT_TRUE(tag.has_value());
    EXPECT_TRUE(tag->equivalentTo(type::signedInteger()));
}

TEST(ParseEnvironment, endEnumDefinitionRegistersLargeTag) {
    LexicalSession session;
    ParseEnvironment env{session};
    env.addEnumerator("A", type::fromHostLong(0x100000000L));
    env.addEnumerator("B");
    auto underlying = env.endEnumDefinition("E");
    EXPECT_TRUE(underlying.equivalentTo(type::signedLong()));
    auto tag = env.lookupEnumTag("E");
    ASSERT_TRUE(tag.has_value());
    EXPECT_TRUE(tag->equivalentTo(type::signedLong()));
    type::IntegerConstant v;
    EXPECT_TRUE(env.lookupEnumConstant("B", v));
    EXPECT_EQ(type::toHostLong(v), 0x100000001L);
}

TEST(ParseEnvironment, endEnumDefinitionAnonymousDoesNotRegisterTag) {
    LexicalSession session;
    ParseEnvironment env{session};
    env.addEnumerator("A", type::fromHostLong(1));
    auto underlying = env.endEnumDefinition();
    EXPECT_TRUE(underlying.equivalentTo(type::signedInteger()));
    EXPECT_FALSE(env.lookupEnumTag("").has_value());
}

TEST(ParseEnvironment, nestedLookupEnumTagFindsParent) {
    LexicalSession session;
    ParseEnvironment parent{session};
    parent.addEnumerator("A", type::fromHostLong(0x80000000L));
    parent.endEnumDefinition("U");
    ParseEnvironment nested{session, parent};
    auto tag = nested.lookupEnumTag("U");
    ASSERT_TRUE(tag.has_value());
    EXPECT_TRUE(tag->equivalentTo(type::unsignedInteger()));
}

TEST(ParseEnvironment, enumeratorRedefinitionThrows) {
    LexicalSession session;
    ParseEnvironment env{session};
    env.addEnumerator("A", type::fromHostLong(1));
    EXPECT_THROW(env.addEnumerator("A", type::fromHostLong(1)), std::runtime_error);
    EXPECT_THROW(env.addEnumerator("A", type::fromHostLong(2)), std::runtime_error);
}

TEST(ParseEnvironment, innerBlockMayReuseOuterEnumeratorName) {
    LexicalSession session;
    ParseEnvironment env{session};
    env.addEnumerator("A", type::fromHostLong(1));
    session.enterBlock();
    env.addEnumerator("A", type::fromHostLong(2));
    type::IntegerConstant v;
    ASSERT_TRUE(env.lookupEnumConstant("A", v));
    EXPECT_EQ(type::toHostLong(v), 2);
    session.leaveBlock();
    ASSERT_TRUE(env.lookupEnumConstant("A", v));
    EXPECT_EQ(type::toHostLong(v), 1);
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
    EXPECT_TRUE(session.names.hasTypedef("myint"));
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
    EXPECT_FALSE(session.names.hasTypedef("myint"));
}

TEST(ParseEnvironment, registerInitializedDeclarationShadowsObjectReuse) {
    LexicalSession session;
    ParseEnvironment env{session};
    env.defineTypedef("T", type::signedInteger());
    DeclarationSpecifiers specs { TypeSpecifier { type::signedInteger(), "int" } };
    std::vector<std::unique_ptr<InitializedDeclarator>> decls;
    decls.push_back(plainDeclarator("T"));
    env.registerInitializedDeclaration(specs, decls);
    EXPECT_TRUE(session.names.isIdentifierShadow("T"));
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
    EXPECT_FALSE(session.names.isIdentifierShadow("T"));
    session.enterBlock();
    EXPECT_TRUE(session.names.isIdentifierShadow("T"));
}

TEST(ParseEnvironment, maybeRegisterParameterShadowNoopsForEmptyOrUnknown) {
    LexicalSession session;
    ParseEnvironment env{session};
    env.defineTypedef("T", type::signedInteger());
    env.maybeRegisterParameterShadow("");
    env.maybeRegisterParameterShadow("not_a_typedef");
    session.enterBlock();
    EXPECT_FALSE(session.names.isIdentifierShadow("T"));
    EXPECT_FALSE(session.names.isIdentifierShadow("not_a_typedef"));
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

    StringLiteralExpression literal { "\":\"", ctx };
    auto literalType = env.typeOf(literal);
    ASSERT_TRUE(literalType.has_value());
    EXPECT_TRUE(literalType->isArray());
    EXPECT_EQ(literalType->getArraySize(), 2);
    EXPECT_TRUE(literalType->getElementType().equivalentTo(type::signedCharacter()));

    UnaryExpression deref {
            "*",
            std::make_unique<IdentifierExpression>("p", ctx) };
    auto derefType = env.typeOf(deref);
    ASSERT_TRUE(derefType.has_value());
    EXPECT_TRUE(derefType->equivalentTo(type::signedInteger()));

    UnaryExpression addr {
            "&",
            std::make_unique<IdentifierExpression>("x", ctx) };
    auto addrType = env.typeOf(addr);
    ASSERT_TRUE(addrType.has_value());
    EXPECT_TRUE(addrType->isPointer());
    EXPECT_TRUE(addrType->dereference().equivalentTo(type::signedInteger()));

    UnaryExpression arrayDeref {
            "*",
            std::make_unique<IdentifierExpression>("a", ctx) };
    auto elementType = env.typeOf(arrayDeref);
    ASSERT_TRUE(elementType.has_value());
    EXPECT_TRUE(elementType->equivalentTo(type::signedCharacter()));

    UnaryExpression plus {
            "+",
            std::make_unique<IdentifierExpression>("x", ctx) };
    auto plusType = env.typeOf(plus);
    ASSERT_TRUE(plusType.has_value());
    EXPECT_TRUE(plusType->equivalentTo(type::signedInteger()));

    PrefixExpression prefix {
            "++",
            std::make_unique<IdentifierExpression>("x", ctx) };
    auto prefixType = env.typeOf(prefix);
    ASSERT_TRUE(prefixType.has_value());
    EXPECT_TRUE(prefixType->equivalentTo(type::signedInteger()));

    PostfixExpression postfix {
            std::make_unique<IdentifierExpression>("x", ctx),
            "++" };
    auto postfixType = env.typeOf(postfix);
    ASSERT_TRUE(postfixType.has_value());
    EXPECT_TRUE(postfixType->equivalentTo(type::signedInteger()));

    ArrayAccess arrayIndex {
            std::make_unique<IdentifierExpression>("a", ctx),
            std::make_unique<ConstantExpression>(Constant { "0", type::signedInteger(), ctx }) };
    auto indexType = env.typeOf(arrayIndex);
    ASSERT_TRUE(indexType.has_value());
    EXPECT_TRUE(indexType->equivalentTo(type::signedCharacter()));

    UnaryExpression addrOfElement {
            "&",
            std::make_unique<ArrayAccess>(
                    std::make_unique<IdentifierExpression>("a", ctx),
                    std::make_unique<ConstantExpression>(Constant { "0", type::signedInteger(), ctx })) };
    auto addrOfElementType = env.typeOf(addrOfElement);
    ASSERT_TRUE(addrOfElementType.has_value());
    EXPECT_TRUE(addrOfElementType->isPointer());
    EXPECT_TRUE(addrOfElementType->dereference().equivalentTo(type::signedCharacter()));
}

TEST(ParseEnvironment, typeOfMemberAccess) {
    LexicalSession session;
    ParseEnvironment env{session};
    translation_unit::Context ctx { "t", 1 };

    type::Type rec = type::structure({
            { "x", type::signedInteger() },
            { "items", type::pointer(type::signedInteger()) },
    });
    env.defineObject("s", rec);
    env.defineObject("ps", type::pointer(rec));
    env.defineObject("arr", type::array(rec, 2));
    env.defineObject("i", type::signedInteger());

    MemberAccess dot {
            std::make_unique<IdentifierExpression>("s", ctx),
            "x",
            false,
            ctx };
    auto dotType = env.typeOf(dot);
    ASSERT_TRUE(dotType.has_value());
    EXPECT_TRUE(dotType->equivalentTo(type::signedInteger()));

    MemberAccess arrow {
            std::make_unique<IdentifierExpression>("ps", ctx),
            "x",
            true,
            ctx };
    auto arrowType = env.typeOf(arrow);
    ASSERT_TRUE(arrowType.has_value());
    EXPECT_TRUE(arrowType->equivalentTo(type::signedInteger()));

    MemberAccess arrowItems {
            std::make_unique<IdentifierExpression>("ps", ctx),
            "items",
            true,
            ctx };
    auto itemsType = env.typeOf(arrowItems);
    ASSERT_TRUE(itemsType.has_value());
    EXPECT_TRUE(itemsType->equivalentTo(type::pointer(type::signedInteger())));

    MemberAccess arrayArrow {
            std::make_unique<IdentifierExpression>("arr", ctx),
            "x",
            true,
            ctx };
    auto arrayArrowType = env.typeOf(arrayArrow);
    ASSERT_TRUE(arrayArrowType.has_value());
    EXPECT_TRUE(arrayArrowType->equivalentTo(type::signedInteger()));

    UnaryExpression derefItems {
            "*",
            std::make_unique<MemberAccess>(
                    std::make_unique<IdentifierExpression>("ps", ctx),
                    "items",
                    true,
                    ctx) };
    auto derefItemsType = env.typeOf(derefItems);
    ASSERT_TRUE(derefItemsType.has_value());
    EXPECT_TRUE(derefItemsType->equivalentTo(type::signedInteger()));

    MemberAccess missing {
            std::make_unique<IdentifierExpression>("s", ctx),
            "nope",
            false,
            ctx };
    EXPECT_FALSE(env.typeOf(missing).has_value());

    MemberAccess arrowOnInt {
            std::make_unique<IdentifierExpression>("i", ctx),
            "x",
            true,
            ctx };
    EXPECT_FALSE(env.typeOf(arrowOnInt).has_value());
}

TEST(ParseEnvironment, typeOfPointerArithmetic) {
    LexicalSession session;
    ParseEnvironment env{session};
    translation_unit::Context ctx { "t", 1 };

    env.defineObject("p", type::pointer(type::signedInteger()));
    env.defineObject("q", type::pointer(type::signedInteger()));
    env.defineObject("i", type::signedInteger());
    env.defineObject("a", type::array(type::signedInteger(), 4));
    env.defineObject("b", type::signedInteger());
    env.defineObject("c", type::signedCharacter());

    IdentifierExpression bareArray { "a", ctx };
    auto bareArrayType = env.typeOf(bareArray);
    ASSERT_TRUE(bareArrayType.has_value());
    EXPECT_TRUE(bareArrayType->isArray());
    EXPECT_TRUE(bareArrayType->equivalentTo(type::array(type::signedInteger(), 4)));

    ArithmeticExpression ptrPlus {
            std::make_unique<IdentifierExpression>("p", ctx),
            "+",
            std::make_unique<IdentifierExpression>("i", ctx) };
    auto ptrPlusType = env.typeOf(ptrPlus);
    ASSERT_TRUE(ptrPlusType.has_value());
    EXPECT_TRUE(ptrPlusType->equivalentTo(type::pointer(type::signedInteger())));

    ArithmeticExpression intPlusPtr {
            std::make_unique<IdentifierExpression>("i", ctx),
            "+",
            std::make_unique<IdentifierExpression>("p", ctx) };
    auto intPlusPtrType = env.typeOf(intPlusPtr);
    ASSERT_TRUE(intPlusPtrType.has_value());
    EXPECT_TRUE(intPlusPtrType->equivalentTo(type::pointer(type::signedInteger())));

    ArithmeticExpression ptrMinusInt {
            std::make_unique<IdentifierExpression>("p", ctx),
            "-",
            std::make_unique<IdentifierExpression>("i", ctx) };
    auto ptrMinusIntType = env.typeOf(ptrMinusInt);
    ASSERT_TRUE(ptrMinusIntType.has_value());
    EXPECT_TRUE(ptrMinusIntType->equivalentTo(type::pointer(type::signedInteger())));

    ArithmeticExpression ptrMinusPtr {
            std::make_unique<IdentifierExpression>("p", ctx),
            "-",
            std::make_unique<IdentifierExpression>("q", ctx) };
    auto ptrMinusPtrType = env.typeOf(ptrMinusPtr);
    ASSERT_TRUE(ptrMinusPtrType.has_value());
    EXPECT_TRUE(ptrMinusPtrType->equivalentTo(type::signedInteger()));

    ArithmeticExpression arrPlus {
            std::make_unique<IdentifierExpression>("a", ctx),
            "+",
            std::make_unique<IdentifierExpression>("i", ctx) };
    auto arrPlusType = env.typeOf(arrPlus);
    ASSERT_TRUE(arrPlusType.has_value());
    EXPECT_TRUE(arrPlusType->equivalentTo(type::pointer(type::signedInteger())));

    ArithmeticExpression ptrPlusConst {
            std::make_unique<IdentifierExpression>("p", ctx),
            "+",
            std::make_unique<ConstantExpression>(Constant { "1", type::signedInteger(), ctx }) };
    auto ptrPlusConstType = env.typeOf(ptrPlusConst);
    ASSERT_TRUE(ptrPlusConstType.has_value());
    EXPECT_TRUE(ptrPlusConstType->equivalentTo(type::pointer(type::signedInteger())));

    UnaryExpression derefPtrPlus {
            "*",
            std::make_unique<ArithmeticExpression>(
                    std::make_unique<IdentifierExpression>("p", ctx),
                    "+",
                    std::make_unique<IdentifierExpression>("i", ctx)) };
    auto derefPtrPlusType = env.typeOf(derefPtrPlus);
    ASSERT_TRUE(derefPtrPlusType.has_value());
    EXPECT_TRUE(derefPtrPlusType->equivalentTo(type::signedInteger()));

    ArithmeticExpression intPlus {
            std::make_unique<IdentifierExpression>("i", ctx),
            "+",
            std::make_unique<IdentifierExpression>("b", ctx) };
    auto intPlusType = env.typeOf(intPlus);
    ASSERT_TRUE(intPlusType.has_value());
    EXPECT_TRUE(intPlusType->equivalentTo(type::signedInteger()));

    ArithmeticExpression charPlus {
            std::make_unique<IdentifierExpression>("c", ctx),
            "+",
            std::make_unique<IdentifierExpression>("c", ctx) };
    auto charPlusType = env.typeOf(charPlus);
    ASSERT_TRUE(charPlusType.has_value());
    EXPECT_TRUE(charPlusType->equivalentTo(type::signedInteger()));

    ArithmeticExpression intMul {
            std::make_unique<IdentifierExpression>("i", ctx),
            "*",
            std::make_unique<IdentifierExpression>("b", ctx) };
    auto intMulType = env.typeOf(intMul);
    ASSERT_TRUE(intMulType.has_value());
    EXPECT_TRUE(intMulType->equivalentTo(type::signedInteger()));
}

TEST(ParseEnvironment, typeOfGitShapedDerefMemberPlus) {
    LexicalSession session;
    ParseEnvironment env{session};
    translation_unit::Context ctx { "t", 1 };

    type::Type rec = type::structure({
            { "items", type::pointer(type::signedInteger()) },
    });
    env.defineObject("ps", type::pointer(rec));
    env.defineObject("i", type::signedInteger());

    UnaryExpression gitShape {
            "*",
            std::make_unique<ArithmeticExpression>(
                    std::make_unique<MemberAccess>(
                            std::make_unique<IdentifierExpression>("ps", ctx),
                            "items",
                            true,
                            ctx),
                    "+",
                    std::make_unique<IdentifierExpression>("i", ctx)) };
    auto gitShapeType = env.typeOf(gitShape);
    ASSERT_TRUE(gitShapeType.has_value());
    EXPECT_TRUE(gitShapeType->equivalentTo(type::signedInteger()));
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

TEST(ParseEnvironment, tryDefineObjectDefinesFunction) {
    LexicalSession session;
    ParseEnvironment env{session};
    DeclarationSpecifiers specs { TypeSpecifier { type::signedInteger(), "int" } };
    auto declarator = std::make_unique<Declarator>(
            std::make_unique<FunctionDeclarator>(std::make_unique<Identifier>(
                    TerminalSymbol { "id", "cb", { "t", 1 } })));
    env.tryDefineObject(specs, *declarator);
    auto t = env.lookupObject("cb");
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(t->isFunction());
}

TEST(ParseEnvironment, tryDefineObjectSkipsIncompleteParam) {
    LexicalSession session;
    ParseEnvironment env{session};
    translation_unit::Context ctx { "t", 1 };
    FormalArguments args;
    args.push_back(FormalArgument {
            DeclarationSpecifiers { TypeSpecifier { type::voidType(), "void" } },
            std::make_unique<Declarator>(std::make_unique<ArrayDeclarator>(
                    std::make_unique<Identifier>(TerminalSymbol { "id", "a", ctx }),
                    std::make_unique<ConstantExpression>(
                            Constant { "3", type::signedInteger(), ctx }))) });
    DeclarationSpecifiers specs { TypeSpecifier { type::signedInteger(), "int" } };
    auto declarator = std::make_unique<Declarator>(std::make_unique<FunctionDeclarator>(
            std::make_unique<Identifier>(TerminalSymbol { "id", "f", ctx }), std::move(args)));
    EXPECT_NO_THROW(env.tryDefineObject(specs, *declarator));
    EXPECT_FALSE(env.lookupObject("f").has_value());
}

TEST(ParseEnvironment, tryDefineObjectSkipsPendingTypeof) {
    LexicalSession session;
    ParseEnvironment env{session};
    DeclarationSpecifiers specs { TypeSpecifier {
            std::make_shared<IdentifierExpression>("nope", translation_unit::Context { "t", 1 }) } };
    ASSERT_TRUE(specs.needsSemanticResolve());
    auto declarator = std::make_unique<Declarator>(
            std::make_unique<FunctionDeclarator>(std::make_unique<Identifier>(
                    TerminalSymbol { "id", "cb", { "t", 1 } })));
    env.tryDefineObject(specs, *declarator);
    EXPECT_FALSE(env.lookupObject("cb").has_value());
}

TEST(ParseEnvironment, registerInitializedDeclarationSkipsIncompleteParam) {
    LexicalSession session;
    ParseEnvironment env{session};
    translation_unit::Context ctx { "t", 1 };
    FormalArguments args;
    args.push_back(FormalArgument {
            DeclarationSpecifiers { TypeSpecifier { type::voidType(), "void" } },
            std::make_unique<Declarator>(std::make_unique<ArrayDeclarator>(
                    std::make_unique<Identifier>(TerminalSymbol { "id", "a", ctx }),
                    std::make_unique<ConstantExpression>(
                            Constant { "3", type::signedInteger(), ctx }))) });
    DeclarationSpecifiers specs { TypeSpecifier { type::signedInteger(), "int" } };
    std::vector<std::unique_ptr<InitializedDeclarator>> decls;
    decls.push_back(std::make_unique<InitializedDeclarator>(std::make_unique<Declarator>(
            std::make_unique<FunctionDeclarator>(
                    std::make_unique<Identifier>(TerminalSymbol { "id", "f", ctx }), std::move(args)))));
    EXPECT_NO_THROW(env.registerInitializedDeclaration(specs, decls));
    EXPECT_FALSE(env.lookupObject("f").has_value());
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
