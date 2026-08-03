#include "gtest/gtest.h"

#include "types/IntegerConstant.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"

#include <optional>
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

TEST(TypeQuery, bareAndPointerToFunction) {
    type::Type fn = type::function(type::signedInteger(), {});
    type::Type pfn = type::pointer(fn);
    type::Type ppfn = type::pointer(pfn);

    EXPECT_TRUE(type::isBareFunction(fn));
    EXPECT_FALSE(type::isBareFunction(pfn));
    EXPECT_FALSE(pfn.isFunction());
    EXPECT_TRUE(type::isPointerToFunction(pfn));
    EXPECT_FALSE(type::isPointerToFunction(ppfn));
    EXPECT_TRUE(type::isPointerToFunction(ppfn.dereference()));
}

TEST(TypeQuery, incompleteObjectType) {
    EXPECT_TRUE(type::isIncompleteObjectType(type::voidType()));
    EXPECT_TRUE(type::isIncompleteObjectType(type::function(type::signedInteger(), {})));
    EXPECT_TRUE(type::isIncompleteObjectType(type::incompleteStructure()));
    EXPECT_TRUE(type::isIncompleteObjectType(type::incompleteArray(type::signedInteger())));
    EXPECT_FALSE(type::isIncompleteObjectType(type::array(type::signedInteger(), 0)));
    EXPECT_FALSE(type::isIncompleteObjectType(type::variableArray(type::signedInteger())));
    EXPECT_FALSE(type::isIncompleteObjectType(type::signedInteger()));
    EXPECT_FALSE(type::isIncompleteObjectType(type::pointer(type::voidType())));
}

TEST(TypeQuery, variableArrayHasRuntimeSizePointerDoesNot) {
    type::Type va = type::variableArray(type::signedInteger());
    type::Type nested = type::array(va, 2);
    type::Type pva = type::pointer(va);
    EXPECT_TRUE(type::hasRuntimeSize(va));
    EXPECT_TRUE(type::hasRuntimeSize(nested));
    EXPECT_FALSE(type::hasRuntimeSize(pva));
    EXPECT_TRUE(type::isVariablyModified(pva));
    EXPECT_FALSE(type::hasRuntimeSize(type::array(type::signedInteger(), 3)));
    EXPECT_EQ(type::sizeofObject(va, false), std::nullopt);
    EXPECT_EQ(type::sizeofObject(nested, false), std::nullopt);
    EXPECT_EQ(type::sizeofObject(pva, false), 8);
}

TEST(TypeQuery, recordWithVlaMemberStaysTentative) {
    type::Type rec = type::incompleteRecord();
    type::completeStructure(rec, { type::MemberSpec { "a", type::variableArray(type::signedInteger()) } });
    EXPECT_TRUE(type::isTentativeRecord(rec));
    EXPECT_TRUE(rec.isIncompleteRecord());
    EXPECT_FALSE(type::isTentativeRecord(type::incompleteRecord()));
    EXPECT_EQ(type::sizeofObject(rec, false), std::nullopt);
}

TEST(TypeQuery, recordWithFixedArrayMemberIsComplete) {
    type::Type rec = type::incompleteRecord();
    type::completeStructure(rec, {
            type::MemberSpec { "a", type::array(type::pointer(type::voidType()), 6) } });
    EXPECT_FALSE(type::isTentativeRecord(rec));
    EXPECT_TRUE(rec.isCompleteRecord());
    EXPECT_EQ(type::sizeofObject(rec, false), 48);
}

TEST(TypeQuery, unspecifiedVlaHasNoComputableRuntimeSize) {
    type::Type unspecified = type::variableArray(type::signedInteger());
    type::Type nested = type::array(unspecified, 2);
    type::Type pointerTo = type::pointer(unspecified);
    EXPECT_TRUE(type::hasUnspecifiedVlaSize(unspecified));
    EXPECT_TRUE(type::hasUnspecifiedVlaSize(nested));
    EXPECT_FALSE(type::hasUnspecifiedVlaSize(pointerTo));
    EXPECT_FALSE(type::hasComputableRuntimeSize(unspecified));
    EXPECT_FALSE(type::hasComputableRuntimeSize(type::array(type::signedInteger(), 3)));
    EXPECT_FALSE(type::hasUnspecifiedVlaSize(type::array(type::signedInteger(), 3)));
}

TEST(TypeQuery, sizeofObjectGnuFunctionIsOne) {
    type::Type fn = type::function(type::signedInteger(), {});
    EXPECT_EQ(type::sizeofObject(fn, true), 1);
    EXPECT_EQ(type::sizeofObject(fn, false), std::nullopt);
    EXPECT_EQ(type::sizeofObject(type::signedInteger(), true), 4);
    EXPECT_EQ(type::sizeofObject(type::signedInteger(), false), 4);
    EXPECT_EQ(type::sizeofObject(type::voidType(), true), std::nullopt);
    EXPECT_EQ(type::sizeofObject(type::pointer(fn), true), 8);
}

TEST(TypeQuery, incompletePredicatesShareDefinition) {
    type::Type inc = type::incompleteStructure();
    EXPECT_EQ(type::isIncompleteObjectType(inc), type::isIncompleteMemberOrElementType(inc));
    EXPECT_EQ(type::isIncompleteObjectType(type::voidType()),
            type::isIncompleteMemberOrElementType(type::voidType()));
    EXPECT_EQ(type::isIncompleteObjectType(type::signedInteger()),
            type::isIncompleteMemberOrElementType(type::signedInteger()));
}

TEST(TypeQuery, classifyPointerArithmeticForms) {
    type::Type i = type::signedInteger();
    type::Type pi = type::pointer(i);

    auto none = type::classifyPointerArithmetic(i, i, '+');
    EXPECT_EQ(none.form, type::PointerArithmeticForm::None);

    auto ppi = type::classifyPointerArithmetic(pi, i, '+');
    EXPECT_EQ(ppi.form, type::PointerArithmeticForm::PtrPlusInt);
    EXPECT_TRUE(ppi.resultType.isPointer());
    EXPECT_EQ(ppi.strideBytes, 4);

    auto ipp = type::classifyPointerArithmetic(i, pi, '+');
    EXPECT_EQ(ipp.form, type::PointerArithmeticForm::IntPlusPtr);

    auto pmi = type::classifyPointerArithmetic(pi, i, '-');
    EXPECT_EQ(pmi.form, type::PointerArithmeticForm::PtrMinusInt);

    auto pmp = type::classifyPointerArithmetic(pi, pi, '-');
    EXPECT_EQ(pmp.form, type::PointerArithmeticForm::PtrMinusPtr);
    EXPECT_TRUE(pmp.resultType.isPrimitive());

    auto inv = type::classifyPointerArithmetic(pi, pi, '+');
    EXPECT_EQ(inv.form, type::PointerArithmeticForm::Invalid);
}

TEST(TypeQuery, classifyPointerToVlaIsPtrPlusInt) {
    type::Type pva = type::pointer(type::variableArray(type::signedInteger()));
    auto info = type::classifyPointerArithmetic(pva, type::signedInteger(), '+');
    EXPECT_EQ(info.form, type::PointerArithmeticForm::PtrPlusInt);
    EXPECT_TRUE(pva.dereference().isVariableArray());
}

TEST(TypeQuery, arraySubscriptInfoEmptyCompleteElementIsValid) {
    type::Type empty = type::structure({});
    type::Type arr = type::array(empty, 3);
    auto info = type::arraySubscriptInfo(arr);
    EXPECT_TRUE(info.valid());
    EXPECT_TRUE(info.baseIsArray);
    EXPECT_EQ(info.elementStride, 0);

    type::Type ptr = type::pointer(empty);
    auto pinfo = type::arraySubscriptInfo(ptr);
    EXPECT_TRUE(pinfo.valid());
    EXPECT_FALSE(pinfo.baseIsArray);
    EXPECT_EQ(pinfo.elementStride, 0);
}

TEST(TypeQuery, productCanAssignScalarsAndPointers) {
    type::Type i = type::signedInteger();
    type::Type pi = type::pointer(i);
    EXPECT_TRUE(type::productAssignFrom(i, i));
    EXPECT_TRUE(type::productAssignFrom(pi, pi));
    EXPECT_TRUE(type::productAssignFrom(pi, i)); // null constant int → pointer
    EXPECT_TRUE(type::productAssignFrom(i, pi)); // product-loose scalar
}

TEST(TypeQuery, productCanAssignFunctionPointer) {
    type::Type fn = type::function(type::signedInteger(), {});
    type::Type pfn = type::pointer(fn);
    type::Type i = type::signedInteger();
    EXPECT_TRUE(type::productAssignFrom(pfn, fn));
    EXPECT_TRUE(type::productAssignFrom(pfn, pfn));
    EXPECT_TRUE(type::productAssignFrom(pfn, i)); // null
    EXPECT_FALSE(type::productAssignFrom(i, fn));
    EXPECT_FALSE(type::productAssignFrom(i, pfn));
    // Type-only gate: void* is not a null pointer constant (SA needs the expression).
    EXPECT_FALSE(type::productAssignFrom(pfn, type::pointer(type::voidType())));
    std::string msg = type::productAssignFailureMessage(i, fn);
    EXPECT_NE(msg.find("function"), std::string::npos);
}

TEST(TypeQuery, adjustedParameterTypeArrayAndFunction) {
    type::Type arr = type::array(type::signedInteger(), 4);
    type::Type adjustedArr = type::adjustedParameterType(arr);
    EXPECT_TRUE(adjustedArr.isPointer());
    EXPECT_TRUE(adjustedArr.dereference().equivalentTo(type::signedInteger()));

    type::Type fn = type::function(type::signedInteger(), { type::signedInteger() });
    type::Type adjustedFn = type::adjustedParameterType(fn);
    EXPECT_TRUE(type::isPointerToFunction(adjustedFn));
    EXPECT_TRUE(adjustedFn.dereference().equivalentTo(fn));

    type::Type i = type::signedInteger();
    EXPECT_TRUE(type::adjustedParameterType(i).equivalentTo(i));
}

TEST(TypeQuery, productCanAssignStructures) {
    auto s = type::structure({ { "x", type::signedInteger() } });
    auto t = type::structure({ { "y", type::signedInteger() } });
    EXPECT_TRUE(type::productAssignFrom(s, s));
    // Distinct structure types (different body identity) do not assign.
    EXPECT_FALSE(type::productAssignFrom(s, t));
    EXPECT_FALSE(type::productAssignFrom(s, type::signedInteger()));
    EXPECT_FALSE(type::productAssignFrom(type::signedInteger(), s));
}

TEST(TypeQuery, productRejectsArrayAndVoidAndIncomplete) {
    type::Type arr = type::array(type::signedInteger(), 3);
    type::Type i = type::signedInteger();
    type::Type pi = type::pointer(i);
    EXPECT_FALSE(type::productAssignFrom(arr, i));
    EXPECT_TRUE(type::productAssignFrom(pi, arr));
    EXPECT_TRUE(type::productAssignFrom(i, arr));
    EXPECT_FALSE(type::productAssignFrom(type::voidType(), i));
    EXPECT_FALSE(type::productAssignFrom(type::incompleteStructure(), i));
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
    EXPECT_FALSE(needsNumericConvert(signedInteger(), pointer(voidType())));
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
    EXPECT_TRUE(needsConversion(signedInteger(), pointer(voidType())));
    EXPECT_EQ(toHostLong(convert(fromHostLong(2), boolean())), 1);
    EXPECT_EQ(toHostLong(convert(fromHostLong(0), boolean())), 0);
    EXPECT_EQ(toHostLong(convert(fromHostLong(2), signedInteger())), 2);
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

TEST(TypeQuery, isSubscriptBasePointerAndArray) {
    type::Type i = type::signedInteger();
    type::Type pi = type::pointer(i);
    type::Type arr = type::array(i, 3);
    EXPECT_TRUE(type::isSubscriptBase(pi, pi));
    EXPECT_TRUE(type::isSubscriptBase(arr, arr));
    EXPECT_TRUE(type::isSubscriptBase(arr, pi));
    EXPECT_TRUE(type::isSubscriptBase(i, pi));
    EXPECT_FALSE(type::isSubscriptBase(i, i));
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

TEST(TypeQuery, conditionalResultTypeDecaysArraysAndPointers) {
    using namespace type;
    auto a4 = array(signedInteger(), 4);
    auto a8 = array(signedInteger(), 8);
    auto r = conditionalResultType(a4, a8);
    ASSERT_TRUE(r.has_value());
    EXPECT_TRUE(r->isPointer());
    EXPECT_TRUE(r->dereference().equivalentTo(signedInteger()));

    auto pi = pointer(signedInteger());
    auto pv = pointer(voidType());
    auto rvoid = conditionalResultType(pi, pv);
    ASSERT_TRUE(rvoid.has_value());
    EXPECT_TRUE(rvoid->isPointer());
    EXPECT_TRUE(rvoid->dereference().isVoid());

    auto fn = function(signedInteger());
    auto rfn = conditionalResultType(fn, fn);
    ASSERT_TRUE(rfn.has_value());
    EXPECT_TRUE(isPointerToFunction(*rfn));
}

TEST(TypeQuery, productArithmeticCompatible) {
    EXPECT_TRUE(type::productArithmeticCompatible(type::signedInteger(), type::signedLong()));
    EXPECT_TRUE(type::productArithmeticCompatible(type::floating(), type::doubleFloating()));
    EXPECT_TRUE(type::productArithmeticCompatible(type::complexFloat(), type::floating()));
    EXPECT_TRUE(type::productArithmeticCompatible(type::complexDouble(), type::complexFloat()));
    EXPECT_FALSE(type::productArithmeticCompatible(type::pointer(type::signedInteger()), type::signedInteger()));
    EXPECT_FALSE(type::productArithmeticCompatible(type::signedInteger(), type::voidType()));
}

TEST(TypeQuery, isFloatingAndIntegral) {
    EXPECT_TRUE(type::isIntegral(type::signedInteger()));
    EXPECT_TRUE(type::isFloating(type::floating()));
    EXPECT_FALSE(type::isFloating(type::signedInteger()));
    EXPECT_FALSE(type::isIntegral(type::floating()));
    EXPECT_TRUE(type::isFloat(type::floating()));
    EXPECT_FALSE(type::isFloat(type::doubleFloating()));
    EXPECT_TRUE(type::isDouble(type::doubleFloating()));
    EXPECT_FALSE(type::isDouble(type::longDoubleFloating()));
    EXPECT_TRUE(type::isLongDouble(type::longDoubleFloating()));
    EXPECT_FALSE(type::isLongDouble(type::doubleFloating()));
    EXPECT_TRUE(type::isComplex(type::complexFloat()));
    EXPECT_TRUE(type::isComplexFloat(type::complexFloat()));
    EXPECT_TRUE(type::isComplexDouble(type::complexDouble()));
    EXPECT_TRUE(type::isComplexLongDouble(type::complexLongDouble()));
    EXPECT_FALSE(type::isFloating(type::complexFloat()));
    EXPECT_FALSE(type::isIntegral(type::complexDouble()));
    EXPECT_FALSE(type::isComplex(type::doubleFloating()));
    EXPECT_TRUE(type::isRealType(type::signedInteger()));
    EXPECT_TRUE(type::isRealType(type::floating()));
    EXPECT_TRUE(type::isRealType(type::doubleFloating()));
    EXPECT_TRUE(type::isRealType(type::longDoubleFloating()));
    EXPECT_TRUE(type::isRealType(type::boolean()));
    EXPECT_FALSE(type::isRealType(type::complexFloat()));
    EXPECT_FALSE(type::isRealType(type::complexDouble()));
    EXPECT_FALSE(type::isRealType(type::complexLongDouble()));
    EXPECT_FALSE(type::isRealType(type::pointer(type::signedInteger())));
    EXPECT_FALSE(type::isRealType(type::voidType()));
    EXPECT_TRUE(type::isArithmeticType(type::complexFloat()));
    EXPECT_TRUE(type::isArithmeticType(type::signedInteger()));
    EXPECT_FALSE(type::isArithmeticType(type::pointer(type::signedInteger())));
    EXPECT_TRUE(type::correspondingReal(type::complexFloat()).equivalentTo(type::floating()));
    EXPECT_TRUE(type::correspondingReal(type::complexDouble()).equivalentTo(type::doubleFloating()));
    EXPECT_TRUE(type::correspondingReal(type::complexLongDouble()).equivalentTo(type::longDoubleFloating()));
    EXPECT_TRUE(type::complexOfReal(type::floating()).equivalentTo(type::complexFloat()));
    EXPECT_TRUE(type::complexOfReal(type::doubleFloating()).equivalentTo(type::complexDouble()));
    EXPECT_TRUE(type::complexOfReal(type::longDoubleFloating()).equivalentTo(type::complexLongDouble()));
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

TEST(TypeQuery, assignmentConvertTargetPointerPlusEqualKeepsInteger) {
    type::Type pi = type::pointer(type::signedInteger());
    EXPECT_TRUE(type::assignmentConvertTarget("+=", pi, type::signedInteger())
            .equivalentTo(type::signedInteger()));
    EXPECT_TRUE(type::assignmentConvertTarget("-=", pi, type::signedCharacter())
            .equivalentTo(type::signedInteger()));
    EXPECT_TRUE(type::assignmentConvertTarget("=", pi, type::signedInteger())
            .equivalentTo(pi));
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
    EXPECT_FALSE(type::needsIntegerWiden(type::signedInteger(), type::pointer(type::voidType())));
}

TEST(TypeQuery, needsIntegerToPointerExtend) {
    EXPECT_TRUE(type::needsIntegerToPointerExtend(type::signedInteger(), type::pointer(type::voidType())));
    EXPECT_TRUE(type::needsIntegerToPointerExtend(type::unsignedInteger(), type::pointer(type::voidType())));
    EXPECT_FALSE(type::needsIntegerToPointerExtend(type::signedLong(), type::pointer(type::voidType())));
    EXPECT_FALSE(type::needsIntegerToPointerExtend(type::signedInteger(), type::signedLong()));
}

TEST(TypeQuery, convertScalarConstantTruncatesToDestWidth) {
    EXPECT_EQ(type::toHostLong(type::convert(type::fromHostLong(-1), type::unsignedInteger())),
            0xffffffffL);
    EXPECT_EQ(type::toHostLong(type::convert(
                      type::fromHostLong(static_cast<long>(0x8000000000000000ULL)),
                      type::unsignedInteger())),
            0L);
    EXPECT_EQ(type::toHostLong(type::convert(type::fromHostLong(-1), type::unsignedLong())), -1L);
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

TEST(TypeQuery, usualArithmeticResult) {
    auto r = type::usualArithmeticResult(type::signedCharacter(), type::signedInteger());
    EXPECT_TRUE(r.isPrimitive());
    EXPECT_EQ(r.getSize(), 4);
    auto rf = type::usualArithmeticResult(type::floating(), type::signedInteger());
    EXPECT_TRUE(type::isFloating(rf));
    EXPECT_EQ(rf.getSize(), 4);
    auto rd = type::usualArithmeticResult(type::floating(), type::doubleFloating());
    EXPECT_TRUE(type::isDouble(rd));
    auto rld = type::usualArithmeticResult(type::longDoubleFloating(), type::doubleFloating());
    EXPECT_TRUE(type::isLongDouble(rld));
    auto rldi = type::usualArithmeticResult(type::signedInteger(), type::longDoubleFloating());
    EXPECT_TRUE(type::isLongDouble(rldi));
    auto rcf = type::usualArithmeticResult(type::complexFloat(), type::signedInteger());
    EXPECT_TRUE(type::isComplexFloat(rcf));
    auto rcd = type::usualArithmeticResult(type::complexFloat(), type::doubleFloating());
    EXPECT_TRUE(type::isComplexDouble(rcd));
    auto rcld = type::usualArithmeticResult(type::complexDouble(), type::longDoubleFloating());
    EXPECT_TRUE(type::isComplexLongDouble(rcld));
    auto rcc = type::usualArithmeticResult(type::complexFloat(), type::complexDouble());
    EXPECT_TRUE(type::isComplexDouble(rcc));
    auto rb = type::usualArithmeticResult(type::boolean(), type::signedInteger());
    EXPECT_TRUE(rb.isPrimitive());
    EXPECT_EQ(rb.getSize(), 4);
    EXPECT_FALSE(type::isBoolean(rb));
}

TEST(TypeQuery, productArithmeticIsScalarOnly) {
    EXPECT_TRUE(type::productArithmeticCompatible(type::signedInteger(), type::signedLong()));
    EXPECT_FALSE(type::productArithmeticCompatible(type::signedInteger(), type::array(type::signedInteger(), 2)));
}

TEST(TypeQuery, signednessHelpersAreNotDualsOutsideIntegrals) {
    type::Type p = type::pointer(type::signedInteger());
    EXPECT_TRUE(type::isUnsignedSide(p));
    EXPECT_TRUE(type::valueIsSigned(p));
    auto uar = type::usualArithmeticResult(type::signedInteger(), type::unsignedInteger());
    EXPECT_EQ(uar.getSize(), 4);
    EXPECT_FALSE(type::valueIsSigned(uar));
    auto prom = type::integerPromote(type::unsignedShort());
    EXPECT_EQ(prom.getSize(), 4);
    EXPECT_TRUE(prom.getPrimitive().isSigned());
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

TEST(TypeQuery, selectGenericAssociationPicksTypedArmThenDefault) {
    type::Type i = type::signedInteger();
    type::Type d = type::doubleFloating();
    std::vector<type::GenericArmView> arms {
            { false, &d },
            { false, &i },
            { true, nullptr },
    };
    auto choice = type::selectGenericAssociation(i, arms);
    EXPECT_EQ(choice.status, type::GenericSelectionStatus::Ok);
    ASSERT_TRUE(choice.index.has_value());
    EXPECT_EQ(*choice.index, 1u);

    type::Type c = type::signedCharacter();
    choice = type::selectGenericAssociation(c, arms);
    EXPECT_EQ(choice.status, type::GenericSelectionStatus::Ok);
    ASSERT_TRUE(choice.index.has_value());
    EXPECT_EQ(*choice.index, 2u);
}

TEST(TypeQuery, selectGenericAssociationRejectsMultipleMatches) {
    type::Type i = type::signedInteger();
    std::vector<type::GenericArmView> arms {
            { false, &i },
            { false, &i },
    };
    auto choice = type::selectGenericAssociation(i, arms);
    EXPECT_EQ(choice.status, type::GenericSelectionStatus::MultipleMatches);
    EXPECT_FALSE(choice.index.has_value());
}

TEST(TypeQuery, selectGenericAssociationNoMatchWithoutDefault) {
    type::Type i = type::signedInteger();
    type::Type d = type::doubleFloating();
    std::vector<type::GenericArmView> arms { { false, &d } };
    auto choice = type::selectGenericAssociation(i, arms);
    EXPECT_EQ(choice.status, type::GenericSelectionStatus::NoMatch);
    EXPECT_FALSE(choice.index.has_value());
}

TEST(TypeQuery, packedStructHasNoPadding) {
    auto s = type::incompleteRecord();
    type::completeStructure(s, {
            type::MemberSpec { "c", type::signedCharacter() },
            type::MemberSpec { "i", type::signedInteger() },
    }, true);
    EXPECT_TRUE(s.isPacked());
    EXPECT_EQ(s.getSize(), 5);
    EXPECT_EQ(s.getAlignment(), 1);
    ASSERT_EQ(s.memberCount(), 2);
    EXPECT_EQ(s.getMembers()[0].offsetBytes, 0);
    EXPECT_EQ(s.getMembers()[1].offsetBytes, 1);
}

TEST(TypeQuery, unpackedStructPadsCharThenInt) {
    auto s = type::structure({
            { "c", type::signedCharacter() },
            { "i", type::signedInteger() },
    });
    EXPECT_FALSE(s.isPacked());
    EXPECT_EQ(s.getSize(), 8);
    EXPECT_EQ(s.getAlignment(), 4);
    ASSERT_EQ(s.memberCount(), 2);
    EXPECT_EQ(s.getMembers()[1].offsetBytes, 4);
}

TEST(TypeQuery, applyPackedRelayoutsCompletedStruct) {
    auto s = type::structure({
            { "c", type::signedCharacter() },
            { "i", type::signedInteger() },
    });
    EXPECT_EQ(s.getSize(), 8);
    s.applyPacked();
    EXPECT_TRUE(s.isPacked());
    EXPECT_EQ(s.getSize(), 5);
    EXPECT_EQ(s.getAlignment(), 1);
    EXPECT_EQ(s.getMembers()[1].offsetBytes, 1);
}

TEST(TypeQuery, applyPackedRelayoutsCompletedUnion) {
    auto u = unionType({
            { "c", signedCharacter() },
            { "i", signedInteger() },
    });
    EXPECT_EQ(u.getSize(), 4);
    EXPECT_EQ(u.getAlignment(), 4);
    u.applyPacked();
    EXPECT_TRUE(u.isUnion());
    EXPECT_TRUE(u.isPacked());
    EXPECT_EQ(u.getSize(), 4);
    EXPECT_EQ(u.getAlignment(), 1);
}

TEST(TypeQuery, recompleteRecordKeepsPackedLayout) {
    auto s = incompleteRecord();
    completeStructure(s, {
            MemberSpec { "c", signedCharacter() },
            MemberSpec { "i", signedInteger() },
    }, true);
    recompleteRecord(s, {
            MemberSpec { "c", signedCharacter() },
            MemberSpec { "i", signedInteger() },
            MemberSpec { "a", array(signedCharacter(), 4) },
    });
    EXPECT_TRUE(s.isPacked());
    EXPECT_FALSE(s.isUnion());
    EXPECT_EQ(s.getSize(), 9);
    ASSERT_EQ(s.memberCount(), 3);
    EXPECT_EQ(s.getMembers()[1].offsetBytes, 1);
}

TEST(TypeQuery, recompleteRecordKeepsUnpackedLayout) {
    auto s = structure({
            { "c", signedCharacter() },
            { "i", signedInteger() },
    });
    recompleteRecord(s, {
            MemberSpec { "c", signedCharacter() },
            MemberSpec { "i", signedInteger() },
            MemberSpec { "a", array(signedCharacter(), 4) },
    });
    EXPECT_FALSE(s.isPacked());
    EXPECT_EQ(s.getSize(), 12);
    ASSERT_EQ(s.memberCount(), 3);
    EXPECT_EQ(s.getMembers()[1].offsetBytes, 4);
}

TEST(TypeQuery, recompleteRecordKeepsPackedUnion) {
    auto u = incompleteRecord();
    completeUnion(u, {
            MemberSpec { "c", signedCharacter() },
            MemberSpec { "i", signedInteger() },
    }, true);
    recompleteRecord(u, {
            MemberSpec { "c", signedCharacter() },
            MemberSpec { "i", signedInteger() },
            MemberSpec { "a", array(signedCharacter(), 5) },
    });
    EXPECT_TRUE(u.isUnion());
    EXPECT_TRUE(u.isPacked());
    EXPECT_EQ(u.getSize(), 5);
    EXPECT_EQ(u.getAlignment(), 1);
}

TEST(TypeQuery, packedUnionAlignmentIsOne) {
    auto u = type::incompleteRecord();
    type::completeUnion(u, {
            type::MemberSpec { "c", type::signedCharacter() },
            type::MemberSpec { "i", type::signedInteger() },
    }, true);
    EXPECT_TRUE(u.isPacked());
    EXPECT_TRUE(u.isUnion());
    EXPECT_EQ(u.getSize(), 4);
    EXPECT_EQ(u.getAlignment(), 1);
}

} // namespace


