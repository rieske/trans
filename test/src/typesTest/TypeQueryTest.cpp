#include "gtest/gtest.h"

#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"

#include <string>

namespace {

using namespace type;

TEST(TypeQuery, valueIsSignedIntegralsAndDefaults) {
    EXPECT_TRUE(valueIsSigned(signedInteger()));
    EXPECT_FALSE(valueIsSigned(unsignedInteger()));
    EXPECT_TRUE(valueIsSigned(signedCharacter()));
    EXPECT_FALSE(valueIsSigned(unsignedCharacter()));
    // Non-integrals default signed (stack Value / SAR policy).
    EXPECT_TRUE(valueIsSigned(pointer(signedInteger())));
    EXPECT_TRUE(valueIsSigned(array(signedInteger(), 2)));
    EXPECT_TRUE(valueIsSigned(doubleFloating()));
}

TEST(TypeQuery, memoryAccessIsSignedOnlySignedIntegrals) {
    EXPECT_TRUE(memoryAccessIsSigned(signedInteger()));
    EXPECT_FALSE(memoryAccessIsSigned(unsignedInteger()));
    // Loads through pointers / floats zero-extend / don't sign-extend.
    EXPECT_FALSE(memoryAccessIsSigned(pointer(signedInteger())));
    EXPECT_FALSE(memoryAccessIsSigned(doubleFloating()));
}

TEST(TypeQuery, valueSizeBytesAndWords) {
    EXPECT_EQ(valueSizeBytes(signedInteger()), 4);
    EXPECT_EQ(valueSizeBytes(voidType()), 8); // empty → word home
    EXPECT_EQ(type::object_abi::valueWords(valueSizeBytes(signedInteger())), 1);
    EXPECT_EQ(type::object_abi::valueWords(valueSizeBytes(array(signedInteger(), 3))), 2);
}

TEST(TypeQuery, memoryAccessSizeBytes) {
    EXPECT_EQ(memoryAccessSizeBytes(signedCharacter()), 1);
    EXPECT_EQ(memoryAccessSizeBytes(signedShort()), 2);
    EXPECT_EQ(memoryAccessSizeBytes(signedInteger()), 4);
    EXPECT_EQ(memoryAccessSizeBytes(signedLong()), 8);
    EXPECT_EQ(memoryAccessSizeBytes(pointer(signedInteger())), 8);
}

// Predicates must use kind(), not payload presence: pointer-to-T still carries T.
TEST(TypeQuery, isFloatingIgnoresPointerPayload) {
    EXPECT_TRUE(isFloating(doubleFloating()));
    EXPECT_FALSE(isFloating(pointer(doubleFloating())));
    EXPECT_FALSE(isFloating(signedInteger()));
    EXPECT_TRUE(isFloat(floating()));
    EXPECT_FALSE(isFloat(doubleFloating()));
    EXPECT_TRUE(isDouble(doubleFloating()));
    EXPECT_FALSE(isDouble(longDoubleFloating()));
    EXPECT_TRUE(isLongDouble(longDoubleFloating()));
    EXPECT_FALSE(isLongDouble(doubleFloating()));
    EXPECT_TRUE(isComplex(complexFloat()));
    EXPECT_TRUE(isComplexFloat(complexFloat()));
    EXPECT_TRUE(isComplexDouble(complexDouble()));
    EXPECT_TRUE(isComplexLongDouble(complexLongDouble()));
    EXPECT_FALSE(isFloating(complexFloat()));
    EXPECT_FALSE(isIntegral(complexDouble()));
    EXPECT_FALSE(isComplex(doubleFloating()));
    EXPECT_TRUE(isRealType(signedInteger()));
    EXPECT_TRUE(isRealType(floating()));
    EXPECT_FALSE(isRealType(complexFloat()));
    EXPECT_TRUE(isArithmeticType(complexFloat()));
    EXPECT_TRUE(correspondingReal(complexFloat()).equivalentTo(floating()));
    EXPECT_TRUE(correspondingReal(complexDouble()).equivalentTo(doubleFloating()));
    EXPECT_TRUE(correspondingReal(complexLongDouble()).equivalentTo(longDoubleFloating()));
    EXPECT_TRUE(complexOfReal(floating()).equivalentTo(complexFloat()));
    EXPECT_TRUE(complexOfReal(doubleFloating()).equivalentTo(complexDouble()));
    EXPECT_TRUE(complexOfReal(longDoubleFloating()).equivalentTo(complexLongDouble()));
}

TEST(TypeQuery, isIntegralIgnoresPointerPayload) {
    EXPECT_TRUE(isIntegral(signedInteger()));
    EXPECT_FALSE(isIntegral(pointer(signedInteger())));
    EXPECT_FALSE(isIntegral(doubleFloating()));
}

TEST(TypeQuery, isUnsignedSidePointersAndUnsigned) {
    EXPECT_TRUE(isUnsignedSide(pointer(signedInteger())));
    EXPECT_TRUE(isUnsignedSide(array(signedInteger(), 2)));
    EXPECT_TRUE(isUnsignedSide(unsignedInteger()));
    EXPECT_FALSE(isUnsignedSide(signedInteger()));
    EXPECT_FALSE(isUnsignedSide(doubleFloating()));
}

// Pointer-to-float is never floating for codegen/globals; isPrimitive is false too.
TEST(TypeQuery, pointerToFloatIsNotFloatingForCodegenGlobals) {
    type::Type p = type::pointer(type::doubleFloating());
    EXPECT_FALSE(p.isPrimitive());
    EXPECT_FALSE(type::isFloating(p));
    EXPECT_FALSE(type::isIntegral(p));
    EXPECT_TRUE(type::isUnsignedSide(p)); // pointers are address-sized unsigned side
}

TEST(TypeQuery, integralSignednessForShiftsUsesKind) {
    EXPECT_TRUE(type::isIntegral(type::signedInteger()));
    EXPECT_TRUE(type::valueIsSigned(type::signedInteger()));
    EXPECT_FALSE(type::valueIsSigned(type::unsignedInteger()));
    // Pointer must not look like a signed integral for SAR vs SHR policy.
    EXPECT_FALSE(type::isIntegral(type::pointer(type::signedInteger())));
    EXPECT_TRUE(type::valueIsSigned(type::pointer(type::signedInteger()))); // stack Value default
}

// Usual arithmetic result type: promote, wider wins, same-size unsigned over signed.
TEST(TypeQuery, usualArithmeticResultWiderAndUnsignedWins) {
    EXPECT_TRUE(usualArithmeticResult(signedInteger(), doubleFloating()).equivalentTo(doubleFloating()));
    EXPECT_TRUE(usualArithmeticResult(signedCharacter(), signedInteger()).equivalentTo(signedInteger()));
    // same size: unsigned wins over signed
    EXPECT_TRUE(usualArithmeticResult(signedInteger(), unsignedInteger()).equivalentTo(unsignedInteger()));
    EXPECT_TRUE(usualArithmeticResult(unsignedInteger(), signedInteger()).equivalentTo(unsignedInteger()));
    // wider wins even if signed
    EXPECT_TRUE(usualArithmeticResult(unsignedInteger(), signedLong()).equivalentTo(signedLong()));
}

TEST(TypeQuery, needsNumericConvert) {
    EXPECT_TRUE(needsNumericConvert(floating(), signedInteger()));
    EXPECT_TRUE(needsNumericConvert(floating(), doubleFloating()));
    EXPECT_TRUE(needsNumericConvert(boolean(), floating()));
    EXPECT_TRUE(needsNumericConvert(signedInteger(), signedLong()));
    EXPECT_TRUE(needsNumericConvert(signedLong(), signedInt128()));
    EXPECT_FALSE(needsNumericConvert(floating(), floating()));
    EXPECT_FALSE(needsNumericConvert(signedInt128(), signedLong()));
    EXPECT_FALSE(needsNumericConvert(signedInteger(), boolean()));
    EXPECT_FALSE(needsNumericConvert(floating(), boolean()));
    EXPECT_TRUE(needsNumericConvert(floating(), complexFloat()));
    EXPECT_TRUE(needsNumericConvert(complexFloat(), floating()));
    EXPECT_TRUE(needsNumericConvert(complexFloat(), complexDouble()));
    EXPECT_TRUE(needsNumericConvert(signedInteger(), complexFloat()));
    EXPECT_FALSE(needsNumericConvert(complexFloat(), complexFloat()));
}

TEST(TypeQuery, characterIsNotBoolean) {
    EXPECT_TRUE(isCharacter(signedCharacter()));
    EXPECT_TRUE(isCharacter(unsignedCharacter()));
    EXPECT_FALSE(isCharacter(boolean()));
    EXPECT_FALSE(isCharacter(signedInteger()));
    EXPECT_TRUE(isBoolean(boolean()));
    EXPECT_FALSE(isBoolean(unsignedCharacter()));
}

TEST(TypeQuery, needsBoolConvert) {
    EXPECT_TRUE(needsBoolConvert(signedInteger(), boolean()));
    EXPECT_TRUE(needsBoolConvert(unsignedCharacter(), boolean()));
    EXPECT_TRUE(needsBoolConvert(pointer(signedInteger()), boolean()));
    EXPECT_TRUE(needsBoolConvert(floating(), boolean()));
    EXPECT_FALSE(needsBoolConvert(boolean(), boolean()));
    EXPECT_FALSE(needsBoolConvert(boolean(), signedInteger()));
}

TEST(TypeQuery, needsConversionAndConstantBool) {
    EXPECT_TRUE(needsConversion(signedInteger(), boolean()));
    EXPECT_TRUE(needsConversion(floating(), signedInteger()));
    EXPECT_FALSE(needsConversion(boolean(), boolean()));
    EXPECT_TRUE(needsConversion(signedInteger(), signedLong()));
    EXPECT_TRUE(needsConversion(unsignedInteger(), signedLong()));
    EXPECT_TRUE(needsConversion(signedLong(), signedInt128()));
    EXPECT_TRUE(needsConversion(unsignedLong(), signedInt128()));
    EXPECT_TRUE(needsConversion(signedInteger(), unsignedInt128()));
    EXPECT_FALSE(needsConversion(signedInt128(), signedLong()));
    EXPECT_FALSE(needsConversion(signedLong(), signedLong()));
    EXPECT_FALSE(needsConversion(signedInt128(), unsignedInt128()));
    EXPECT_EQ(convertScalarConstant(boolean(), 2), 1);
    EXPECT_EQ(convertScalarConstant(boolean(), 0), 0);
    EXPECT_EQ(convertScalarConstant(signedInteger(), 2), 2);
}

TEST(TypeQuery, usualArithmeticResultKeepsFloat32UntilDouble) {
    auto rf = usualArithmeticResult(floating(), signedInteger());
    EXPECT_TRUE(isFloating(rf));
    EXPECT_EQ(rf.getSize(), 4);
    auto rd = usualArithmeticResult(floating(), doubleFloating());
    EXPECT_TRUE(isDouble(rd));
    auto rld = usualArithmeticResult(longDoubleFloating(), doubleFloating());
    EXPECT_TRUE(isLongDouble(rld));
    auto rldi = usualArithmeticResult(signedInteger(), longDoubleFloating());
    EXPECT_TRUE(isLongDouble(rldi));
    auto rcf = usualArithmeticResult(complexFloat(), signedInteger());
    EXPECT_TRUE(isComplexFloat(rcf));
    auto rcd = usualArithmeticResult(complexFloat(), doubleFloating());
    EXPECT_TRUE(isComplexDouble(rcd));
    auto rcld = usualArithmeticResult(complexDouble(), longDoubleFloating());
    EXPECT_TRUE(isComplexLongDouble(rcld));
    auto rcc = usualArithmeticResult(complexFloat(), complexDouble());
    EXPECT_TRUE(isComplexDouble(rcc));
    auto rb = usualArithmeticResult(boolean(), signedInteger());
    EXPECT_TRUE(rb.isPrimitive());
    EXPECT_EQ(rb.getSize(), 4);
    EXPECT_FALSE(isBoolean(rb));
}

TEST(TypeQuery, isProductScalarPrimitivesAndPointers) {
    EXPECT_TRUE(isProductScalar(signedInteger()));
    EXPECT_TRUE(isProductScalar(pointer(signedInteger())));
    EXPECT_FALSE(isProductScalar(array(signedInteger(), 2)));
    EXPECT_FALSE(isProductScalar(voidType()));
    auto st = structure({ { "a", signedInteger() } });
    EXPECT_FALSE(isProductScalar(st));
}

TEST(TypeQuery, productAssignFromIsSoleAssignPolicy) {
    EXPECT_TRUE(productAssignFrom(signedLong(), signedInteger()));
    EXPECT_FALSE(productAssignFrom(signedInteger(), structure({ { "a", signedInteger() } })));
    EXPECT_TRUE(productAssignFrom(pointer(signedInteger()), signedInteger())); // null-ish scalar
}

TEST(TypeQuery, arraySubscriptInfoArrayAndPointerBases) {
    auto arr = array(signedInteger(), 4);
    auto infoA = arraySubscriptInfo(arr);
    EXPECT_TRUE(infoA.baseIsArray);
    EXPECT_TRUE(infoA.elementType.equivalentTo(signedInteger()));
    EXPECT_EQ(infoA.elementStride, 4);

    auto ptr = pointer(signedInteger());
    auto infoP = arraySubscriptInfo(ptr);
    EXPECT_FALSE(infoP.baseIsArray);
    EXPECT_TRUE(infoP.elementType.equivalentTo(signedInteger()));
    EXPECT_EQ(infoP.elementStride, 4);

    auto bad = arraySubscriptInfo(signedInteger());
    EXPECT_EQ(bad.elementStride, 0);
}

// Dual-type bases: multi-dim row keeps expression type T[N] while value is T*.
TEST(TypeQuery, arraySubscriptInfoDualTypeMultiDimRow) {
    auto row = array(signedInteger(), 3); // expression type of a[i] for int a[n][3]
    auto decayed = pointer(signedInteger()); // value type after row decay
    auto info = arraySubscriptInfo(row, decayed);
    EXPECT_FALSE(info.baseIsArray); // base is already a pointer value
    EXPECT_TRUE(info.elementType.equivalentTo(signedInteger()));
    EXPECT_EQ(info.elementStride, 4);
    EXPECT_TRUE(info.valid());
}

// Cast / opaque expression type: fall back to value-type pointer.
TEST(TypeQuery, arraySubscriptInfoDualTypeValuePointerFallback) {
    auto info = arraySubscriptInfo(voidType(), pointer(signedCharacter()));
    EXPECT_FALSE(info.baseIsArray);
    EXPECT_TRUE(info.elementType.equivalentTo(signedCharacter()));
    EXPECT_EQ(info.elementStride, 1);
    EXPECT_TRUE(info.valid());
}

TEST(TypeQuery, arraySubscriptInfoDualTypeInvalid) {
    auto info = arraySubscriptInfo(signedInteger(), signedInteger());
    EXPECT_FALSE(info.valid());
    EXPECT_EQ(info.elementStride, 0);
}

TEST(TypeQuery, productAssignFromComposesValueCompatiblePlusFunctionDestReject) {
    // Assign policy is value compatibility except dest must not be a function type.
    EXPECT_TRUE(productAssignFrom(pointer(signedInteger()), signedInteger()));
    EXPECT_EQ(productAssignFrom(signedInteger(), signedLong()),
            productValueCompatible(signedInteger(), signedLong()));
    auto fn = function(signedInteger(), {});
    // Bare function type as dest is rejected before function->pointer decay.
    EXPECT_FALSE(productAssignFrom(fn, signedInteger()));
    // Pointer-to-function dest remains value-compatible with null-ish scalars.
    EXPECT_TRUE(productAssignFrom(pointer(fn), signedInteger()));
}

TEST(TypeQuery, pointerMinusPointerIsSignedLongPtrdiff) {
    auto p = pointer(signedCharacter());
    auto info = classifyPointerArithmetic(p, p, '-');
    EXPECT_EQ(info.form, PointerArithmeticForm::PtrMinusPtr);
    EXPECT_TRUE(info.resultType.equivalentTo(signedLong()));
    EXPECT_EQ(info.strideBytes, 1);
}

TEST(TypeQuery, productArithmeticCompatibleRejectsPointers) {
    auto p = pointer(signedInteger());
    EXPECT_FALSE(productArithmeticCompatible(p, p));
    EXPECT_FALSE(productArithmeticCompatible(p, signedInteger()));
    EXPECT_TRUE(productArithmeticCompatible(signedInteger(), signedLong()));
    EXPECT_TRUE(productArithmeticCompatible(floating(), signedInteger()));
    EXPECT_TRUE(productArithmeticCompatible(complexFloat(), floating()));
    EXPECT_TRUE(productArithmeticCompatible(complexDouble(), complexFloat()));
}

TEST(TypeQuery, memberAccessResultDotAndArrow) {
    type::Type rec = type::structure({
            { "x", type::signedInteger() },
            { "items", type::pointer(type::signedCharacter()) },
    });
    auto dot = type::memberAccessResult(rec, false, "x");
    ASSERT_TRUE(dot.has_value());
    EXPECT_TRUE(dot->equivalentTo(type::signedInteger()));

    auto arrow = type::memberAccessResult(type::pointer(rec), true, "items");
    ASSERT_TRUE(arrow.has_value());
    EXPECT_TRUE(arrow->equivalentTo(type::pointer(type::signedCharacter())));

    auto arrayArrow = type::memberAccessResult(type::array(rec, 2), true, "x");
    ASSERT_TRUE(arrayArrow.has_value());
    EXPECT_TRUE(arrayArrow->equivalentTo(type::signedInteger()));

    EXPECT_FALSE(type::memberAccessResult(rec, false, "nope").has_value());
    EXPECT_FALSE(type::memberAccessResult(type::signedInteger(), true, "x").has_value());
    EXPECT_FALSE(type::memberAccessResult(type::array(rec, 2), false, "x").has_value());
}

TEST(TypeQuery, arithmeticExpressionResultForms) {
    type::Type i = type::signedInteger();
    type::Type c = type::signedCharacter();
    type::Type pi = type::pointer(i);
    type::Type arr = type::array(i, 4);

    auto ptrPlus = type::arithmeticExpressionResult(pi, i, '+');
    ASSERT_TRUE(ptrPlus.has_value());
    EXPECT_TRUE(ptrPlus->equivalentTo(pi));

    auto arrPlus = type::arithmeticExpressionResult(arr, i, '+');
    ASSERT_TRUE(arrPlus.has_value());
    EXPECT_TRUE(arrPlus->equivalentTo(pi));

    auto ptrDiff = type::arithmeticExpressionResult(pi, pi, '-');
    ASSERT_TRUE(ptrDiff.has_value());
    EXPECT_TRUE(ptrDiff->equivalentTo(signedLong()));

    auto uac = type::arithmeticExpressionResult(c, c, '+');
    ASSERT_TRUE(uac.has_value());
    EXPECT_TRUE(uac->equivalentTo(i));

    auto mul = type::arithmeticExpressionResult(i, i, '*');
    ASSERT_TRUE(mul.has_value());
    EXPECT_TRUE(mul->equivalentTo(i));

    EXPECT_FALSE(type::arithmeticExpressionResult(pi, pi, '+').has_value());
    EXPECT_FALSE(type::arithmeticExpressionResult(pi, i, '*').has_value());
}

TEST(TypeQuery, arraySubscriptInfoDualFallbackToValuePointer) {
    // Non-array/non-pointer expression type, pointer value type.
    type::Type expr = type::signedInteger();
    type::Type val = type::pointer(type::signedInteger());
    auto info = type::arraySubscriptInfo(expr, val);
    EXPECT_TRUE(info.valid());
    EXPECT_FALSE(info.baseIsArray);
    EXPECT_TRUE(info.elementType.isPrimitive());
}

TEST(TypeQuery, arraySubscriptInfoDualFallsThroughToExpr) {
    type::Type expr = type::pointer(type::signedInteger());
    type::Type val = type::signedInteger();
    auto info = type::arraySubscriptInfo(expr, val);
    EXPECT_TRUE(info.valid());
    EXPECT_FALSE(info.baseIsArray);
}

TEST(TypeQuery, productValueCompatibleAllowsPointers) {
    auto p = pointer(signedInteger());
    EXPECT_TRUE(productValueCompatible(p, p));
    EXPECT_TRUE(productValueCompatible(p, signedInteger()));
    EXPECT_FALSE(productValueCompatible(signedInteger(), structure({ { "a", signedInteger() } })));
}

TEST(TypeQuery, incompleteMemberOrElement) {
    EXPECT_TRUE(isIncompleteMemberOrElementType(voidType()));
    EXPECT_TRUE(isIncompleteMemberOrElementType(function(signedInteger(), {})));
    EXPECT_TRUE(isIncompleteMemberOrElementType(incompleteStructure()));
    EXPECT_TRUE(isIncompleteMemberOrElementType(incompleteArray(signedInteger())));
    EXPECT_FALSE(isIncompleteMemberOrElementType(pointer(voidType())));
}

TEST(TypeQuery, productAssignFailureMessageTypeMismatch) {
    std::string msg = productAssignFailureMessage(signedInteger(),
            structure({ { "x", signedInteger() } }));
    EXPECT_NE(msg.find("type mismatch"), std::string::npos);
}

TEST(TypeQuery, afterLvalueConversionDropsTopLevelConstAndDecaysArrayAndFunction) {
    auto c = signedInteger({ Qualifier::CONST });
    auto converted = afterLvalueConversion(c);
    EXPECT_FALSE(converted.isConst());
    EXPECT_TRUE(converted.equivalentTo(signedInteger()));

    auto a = array(signedInteger(), 4);
    auto decayed = afterLvalueConversion(a);
    EXPECT_TRUE(decayed.isPointer());
    EXPECT_TRUE(decayed.dereference().equivalentTo(signedInteger()));

    auto f = function(signedInteger());
    auto decayedFn = afterLvalueConversion(f);
    EXPECT_TRUE(decayedFn.isPointer());
    EXPECT_TRUE(decayedFn.dereference().isFunction());
}

TEST(TypeQuery, assignmentConvertTargetPromotesShiftCount) {
    EXPECT_TRUE(type::assignmentConvertTarget("=", type::signedInt128(), type::signedInteger())
            .equivalentTo(type::signedInt128()));
    EXPECT_TRUE(type::assignmentConvertTarget("+=", type::signedInt128(), type::signedInteger())
            .equivalentTo(type::signedInt128()));
    EXPECT_TRUE(type::assignmentConvertTarget("&=", type::signedLong(), type::signedCharacter())
            .equivalentTo(type::signedLong()));
    EXPECT_TRUE(type::assignmentConvertTarget("<<=", type::signedInt128(), type::signedCharacter())
            .equivalentTo(type::signedInteger()));
    EXPECT_TRUE(type::assignmentConvertTarget(">>=", type::signedLong(), type::unsignedCharacter())
            .equivalentTo(type::signedInteger()));
    EXPECT_TRUE(type::assignmentConvertTarget("<<=", type::signedLong(), type::signedInteger())
            .equivalentTo(type::signedInteger()));
}

TEST(TypeQuery, needsIntegerWiden) {
    EXPECT_TRUE(type::needsIntegerWiden(type::signedInteger(), type::signedLong()));
    EXPECT_TRUE(type::needsIntegerWiden(type::unsignedInteger(), type::signedLong()));
    EXPECT_TRUE(type::needsIntegerWiden(type::signedLong(), type::signedInt128()));
    EXPECT_TRUE(type::needsIntegerWiden(type::unsignedLong(), type::unsignedInt128()));
    EXPECT_TRUE(type::needsIntegerWiden(type::signedInteger(), type::unsignedInt128()));
    EXPECT_FALSE(type::needsIntegerWiden(type::signedInt128(), type::signedLong()));
    EXPECT_FALSE(type::needsIntegerWiden(type::signedLong(), type::signedLong()));
    EXPECT_FALSE(type::needsIntegerWiden(type::signedInt128(), type::unsignedInt128()));
    EXPECT_FALSE(type::needsIntegerWiden(type::floating(), type::doubleFloating()));
    EXPECT_FALSE(type::needsIntegerWiden(type::signedInteger(), type::boolean()));
}

TEST(TypeQuery, arraySubscriptPointerToArrayStride) {
    // p is int (*)[3]: p[i] steps by sizeof(int[3])
    Type row = array(signedInteger(), 3);
    Type p = pointer(row);
    auto info = arraySubscriptInfo(p);
    EXPECT_TRUE(info.valid());
    EXPECT_FALSE(info.baseIsArray);
    EXPECT_TRUE(info.elementType.isArray());
    EXPECT_EQ(info.elementStride, 12);
}

TEST(TypeQuery, defaultArgPromote) {
    auto expectInt = [](const Type& t) {
        const Type p = defaultArgPromote(t);
        EXPECT_EQ(p.getSize(), 4);
        EXPECT_TRUE(p.getPrimitive().isSigned());
    };
    expectInt(signedCharacter());
    expectInt(unsignedCharacter());
    expectInt(signedShort());
    expectInt(unsignedShort());
    expectInt(boolean());

    EXPECT_TRUE(defaultArgPromote(signedInteger()).equivalentTo(signedInteger()));
    EXPECT_TRUE(defaultArgPromote(unsignedInteger()).equivalentTo(unsignedInteger()));
    EXPECT_TRUE(defaultArgPromote(signedLong()).equivalentTo(signedLong()));
    EXPECT_TRUE(defaultArgPromote(pointer(signedInteger())).equivalentTo(pointer(signedInteger())));
}

} // namespace


