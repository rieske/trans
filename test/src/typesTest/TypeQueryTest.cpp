#include "gtest/gtest.h"

#include "types/Type.h"
#include "types/TypeQuery.h"

namespace {

TEST(TypeQuery, bareAndPointerToFunction) {
    type::Type fn = type::function(type::signedInteger(), {});
    type::Type pfn = type::pointer(fn);
    type::Type ppfn = type::pointer(pfn);

    EXPECT_TRUE(type::isBareFunction(fn));
    EXPECT_FALSE(type::isBareFunction(pfn));
    // Recursive Type: pointer kind is not function (no payload bleed from pointer()).
    EXPECT_FALSE(pfn.isFunction());
    EXPECT_TRUE(type::isPointerToFunction(pfn));
    // pointer-to-pointer-to-function is not pointer-to-function (old bag model lied here).
    EXPECT_FALSE(type::isPointerToFunction(ppfn));
    EXPECT_TRUE(type::isPointerToFunction(ppfn.dereference()));
    EXPECT_TRUE(type::isPointerToBareFunction(pfn));
    EXPECT_FALSE(type::isPointerToBareFunction(ppfn));
}

TEST(TypeQuery, incompleteObjectType) {
    EXPECT_TRUE(type::isIncompleteObjectType(type::voidType()));
    EXPECT_TRUE(type::isIncompleteObjectType(type::function(type::signedInteger(), {})));
    EXPECT_TRUE(type::isIncompleteObjectType(type::incompleteStructure()));
    EXPECT_TRUE(type::isIncompleteObjectType(type::incompleteArray(type::signedInteger())));
    EXPECT_FALSE(type::isIncompleteObjectType(type::array(type::signedInteger(), 0)));
    EXPECT_FALSE(type::isIncompleteObjectType(type::signedInteger()));
    EXPECT_FALSE(type::isIncompleteObjectType(type::pointer(type::voidType())));
}

TEST(TypeQuery, productCanAssignScalarsAndPointers) {
    type::Type i = type::signedInteger();
    type::Type pi = type::pointer(i);
    EXPECT_TRUE(type::productCanAssignFrom(i, i));
    EXPECT_TRUE(type::productCanAssignFrom(pi, pi));
    EXPECT_TRUE(type::productCanAssignFrom(pi, i)); // null constant int → pointer
    EXPECT_TRUE(type::productCanAssignFrom(i, pi)); // product-loose scalar
}

TEST(TypeQuery, productCanAssignFunctionPointer) {
    type::Type fn = type::function(type::signedInteger(), {});
    type::Type pfn = type::pointer(fn);
    type::Type i = type::signedInteger();
    EXPECT_TRUE(type::productCanAssignFrom(pfn, fn));
    EXPECT_TRUE(type::productCanAssignFrom(pfn, pfn));
    EXPECT_TRUE(type::productCanAssignFrom(pfn, i)); // null
    EXPECT_FALSE(type::productCanAssignFrom(i, fn));
    EXPECT_FALSE(type::productCanAssignFrom(i, pfn));
    // Type-only gate: void* is not a null pointer constant (SA needs the expression).
    EXPECT_FALSE(type::productCanAssignFrom(pfn, type::pointer(type::voidType())));
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
    EXPECT_TRUE(type::productCanAssignFrom(s, s));
    EXPECT_TRUE(type::productCanAssignFrom(s, t)); // product-loose structure-to-structure
    EXPECT_FALSE(type::productCanAssignFrom(s, type::signedInteger()));
    EXPECT_FALSE(type::productCanAssignFrom(type::signedInteger(), s));
}

TEST(TypeQuery, productRejectsArrayAndVoidAndIncomplete) {
    type::Type arr = type::array(type::signedInteger(), 3);
    type::Type i = type::signedInteger();
    type::Type pi = type::pointer(i);
    EXPECT_FALSE(type::productCanAssignFrom(arr, i));
    EXPECT_TRUE(type::productCanAssignFrom(pi, arr));
    EXPECT_TRUE(type::productCanAssignFrom(i, arr));
    EXPECT_FALSE(type::productCanAssignFrom(type::voidType(), i));
    EXPECT_FALSE(type::productCanAssignFrom(type::incompleteStructure(), i));
}

TEST(TypeQuery, arraySubscriptInfoArrayAndPointer) {
    type::Type arr = type::array(type::signedInteger(), 4);
    auto info = type::arraySubscriptInfo(arr);
    EXPECT_TRUE(info.valid());
    EXPECT_TRUE(info.baseIsArray);
    EXPECT_TRUE(info.elementType.isPrimitive());
    // Array-base index steps by sizeof(element), not sizeof(the whole array).
    EXPECT_EQ(info.elementStride, 4);

    type::Type ptr = type::pointer(type::signedInteger());
    auto pinfo = type::arraySubscriptInfo(ptr);
    EXPECT_TRUE(pinfo.valid());
    EXPECT_FALSE(pinfo.baseIsArray);
    EXPECT_EQ(pinfo.elementStride, 4);
}

TEST(TypeQuery, arraySubscriptInfoDualTypeRow) {
    type::Type row = type::array(type::signedInteger(), 3);
    type::Type decayed = type::pointer(type::signedInteger());
    auto info = type::arraySubscriptInfo(row, decayed);
    EXPECT_TRUE(info.valid());
    EXPECT_FALSE(info.baseIsArray);
    EXPECT_TRUE(info.elementType.isPrimitive());
    EXPECT_EQ(info.elementStride, 4);
}

TEST(TypeQuery, arraySubscriptInfoInvalidBase) {
    auto info = type::arraySubscriptInfo(type::signedInteger());
    EXPECT_FALSE(info.valid());
    EXPECT_EQ(info.elementStride, 0);
}

TEST(TypeQuery, arraySubscriptInfoEmptyCompleteElementIsValid) {
    // Empty complete records have size 0; subscript base must still be valid.
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
    EXPECT_TRUE(ptrDiff->equivalentTo(i));

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

TEST(TypeQuery, incompleteMemberOrElement) {
    EXPECT_TRUE(type::isIncompleteMemberOrElementType(type::voidType()));
    EXPECT_TRUE(type::isIncompleteMemberOrElementType(type::function(type::signedInteger(), {})));
    EXPECT_TRUE(type::isIncompleteMemberOrElementType(type::incompleteStructure()));
    EXPECT_TRUE(type::isIncompleteMemberOrElementType(type::incompleteArray(type::signedInteger())));
    EXPECT_FALSE(type::isIncompleteMemberOrElementType(type::pointer(type::voidType())));
}

TEST(TypeQuery, productAssignFailureMessageTypeMismatch) {
    std::string msg = type::productAssignFailureMessage(type::signedInteger(),
            type::structure({ { "x", type::signedInteger() } }));
    EXPECT_NE(msg.find("type mismatch"), std::string::npos);
}


TEST(TypeQuery, productValueCompatibleScalarsAndPointers) {
    type::Type i = type::signedInteger();
    type::Type u = type::unsignedInteger();
    type::Type pi = type::pointer(i);
    EXPECT_TRUE(type::productValueCompatible(i, u));
    EXPECT_TRUE(type::productValueCompatible(pi, pi));
    EXPECT_TRUE(type::productValueCompatible(i, pi)); // decay not required for product-loose
    EXPECT_FALSE(type::productValueCompatible(i, type::voidType()));
}

TEST(TypeQuery, afterLvalueConversionDropsTopLevelConstAndDecaysArrayAndFunction) {
    using namespace type;
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

TEST(TypeQuery, needsNumericConvert) {
    EXPECT_TRUE(type::needsNumericConvert(type::floating(), type::signedInteger()));
    EXPECT_TRUE(type::needsNumericConvert(type::floating(), type::doubleFloating()));
    EXPECT_TRUE(type::needsNumericConvert(type::boolean(), type::floating()));
    EXPECT_TRUE(type::needsNumericConvert(type::signedInteger(), type::signedLong()));
    EXPECT_TRUE(type::needsNumericConvert(type::signedLong(), type::signedInt128()));
    EXPECT_FALSE(type::needsNumericConvert(type::floating(), type::floating()));
    EXPECT_FALSE(type::needsNumericConvert(type::signedInt128(), type::signedLong()));
    EXPECT_FALSE(type::needsNumericConvert(type::signedInteger(), type::boolean()));
    EXPECT_FALSE(type::needsNumericConvert(type::floating(), type::boolean()));
    EXPECT_TRUE(type::needsNumericConvert(type::floating(), type::complexFloat()));
    EXPECT_TRUE(type::needsNumericConvert(type::complexFloat(), type::floating()));
    EXPECT_TRUE(type::needsNumericConvert(type::complexFloat(), type::complexDouble()));
    EXPECT_TRUE(type::needsNumericConvert(type::signedInteger(), type::complexFloat()));
    EXPECT_FALSE(type::needsNumericConvert(type::complexFloat(), type::complexFloat()));
    EXPECT_FALSE(type::needsNumericConvert(type::signedInteger(), type::pointer(type::voidType())));
}

TEST(TypeQuery, characterIsNotBoolean) {
    EXPECT_TRUE(type::isCharacter(type::signedCharacter()));
    EXPECT_TRUE(type::isCharacter(type::unsignedCharacter()));
    EXPECT_FALSE(type::isCharacter(type::boolean()));
    EXPECT_FALSE(type::isCharacter(type::signedInteger()));
    EXPECT_TRUE(type::isBoolean(type::boolean()));
    EXPECT_FALSE(type::isBoolean(type::unsignedCharacter()));
}

TEST(TypeQuery, needsBoolConvert) {
    EXPECT_TRUE(type::needsBoolConvert(type::signedInteger(), type::boolean()));
    EXPECT_TRUE(type::needsBoolConvert(type::unsignedCharacter(), type::boolean()));
    EXPECT_TRUE(type::needsBoolConvert(type::pointer(type::signedInteger()), type::boolean()));
    EXPECT_TRUE(type::needsBoolConvert(type::floating(), type::boolean()));
    EXPECT_FALSE(type::needsBoolConvert(type::boolean(), type::boolean()));
    EXPECT_FALSE(type::needsBoolConvert(type::boolean(), type::signedInteger()));
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
    EXPECT_FALSE(type::needsIntegerWiden(type::signedInteger(), type::pointer(type::voidType())));
}

TEST(TypeQuery, needsIntegerToPointerExtend) {
    EXPECT_TRUE(type::needsIntegerToPointerExtend(type::signedInteger(), type::pointer(type::voidType())));
    EXPECT_TRUE(type::needsIntegerToPointerExtend(type::unsignedInteger(), type::pointer(type::voidType())));
    EXPECT_FALSE(type::needsIntegerToPointerExtend(type::signedLong(), type::pointer(type::voidType())));
    EXPECT_FALSE(type::needsIntegerToPointerExtend(type::signedInteger(), type::signedLong()));
}

TEST(TypeQuery, needsConversionAndConstantBool) {
    EXPECT_TRUE(type::needsConversion(type::signedInteger(), type::boolean()));
    EXPECT_TRUE(type::needsConversion(type::floating(), type::signedInteger()));
    EXPECT_FALSE(type::needsConversion(type::boolean(), type::boolean()));
    EXPECT_TRUE(type::needsConversion(type::signedInteger(), type::signedLong()));
    EXPECT_TRUE(type::needsConversion(type::unsignedInteger(), type::signedLong()));
    EXPECT_TRUE(type::needsConversion(type::signedLong(), type::signedInt128()));
    EXPECT_TRUE(type::needsConversion(type::unsignedLong(), type::signedInt128()));
    EXPECT_TRUE(type::needsConversion(type::signedInteger(), type::unsignedInt128()));
    EXPECT_FALSE(type::needsConversion(type::signedInt128(), type::signedLong()));
    EXPECT_FALSE(type::needsConversion(type::signedLong(), type::signedLong()));
    EXPECT_FALSE(type::needsConversion(type::signedInt128(), type::unsignedInt128()));
    EXPECT_FALSE(type::needsConversion(type::signedInteger(), type::pointer(type::voidType())));
    EXPECT_EQ(type::convertScalarConstant(type::boolean(), 2), 1);
    EXPECT_EQ(type::convertScalarConstant(type::boolean(), 0), 0);
    EXPECT_EQ(type::convertScalarConstant(type::signedInteger(), 2), 2);
}

TEST(TypeQuery, convertScalarConstantTruncatesToDestWidth) {
    EXPECT_EQ(type::convertScalarConstant(type::unsignedInteger(), -1), 0xffffffffL);
    EXPECT_EQ(type::convertScalarConstant(type::unsignedInteger(),
                      static_cast<long>(0x8000000000000000ULL)),
            0L);
    EXPECT_EQ(type::convertScalarConstant(type::unsignedLong(), -1), -1L);
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

TEST(TypeQuery, arraySubscriptPointerToArrayStride) {
    // p is int (*)[3]: p[i] steps by sizeof(int[3])
    type::Type row = type::array(type::signedInteger(), 3);
    type::Type p = type::pointer(row);
    auto info = type::arraySubscriptInfo(p);
    EXPECT_TRUE(info.valid());
    EXPECT_FALSE(info.baseIsArray);
    EXPECT_TRUE(info.elementType.isArray());
    EXPECT_EQ(info.elementStride, 12);
}

TEST(TypeQuery, productAssignRecordsIncludeUnions) {
    auto s = type::structure({ { "x", type::signedInteger() } });
    auto u = type::unionType({ { "y", type::signedInteger() } });
    // Product-loose record-to-record (same policy as structure-to-structure).
    EXPECT_TRUE(type::productAssignFrom(s, s));
    EXPECT_TRUE(type::productAssignFrom(u, u));
    EXPECT_TRUE(type::productAssignFrom(s, u));
    EXPECT_TRUE(type::productAssignFrom(u, s));
    EXPECT_FALSE(type::productAssignFrom(s, type::signedInteger()));
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
    auto expectInt = [](const type::Type& t) {
        const type::Type p = type::defaultArgPromote(t);
        EXPECT_EQ(p.getSize(), 4);
        EXPECT_TRUE(p.getPrimitive().isSigned());
    };
    expectInt(type::signedCharacter());
    expectInt(type::unsignedCharacter());
    expectInt(type::signedShort());
    expectInt(type::unsignedShort());
    expectInt(type::boolean());

    EXPECT_TRUE(type::defaultArgPromote(type::signedInteger()).equivalentTo(type::signedInteger()));
    EXPECT_TRUE(type::defaultArgPromote(type::unsignedInteger()).equivalentTo(type::unsignedInteger()));
    EXPECT_TRUE(type::defaultArgPromote(type::signedLong()).equivalentTo(type::signedLong()));
    EXPECT_TRUE(type::defaultArgPromote(type::pointer(type::signedInteger())).equivalentTo(
            type::pointer(type::signedInteger())));

    EXPECT_TRUE(type::defaultArgPromote(type::floating()).equivalentTo(type::doubleFloating()));
    EXPECT_TRUE(type::defaultArgPromote(type::doubleFloating()).equivalentTo(type::doubleFloating()));
    EXPECT_TRUE(type::defaultArgPromote(type::longDoubleFloating()).equivalentTo(type::longDoubleFloating()));
    EXPECT_TRUE(type::defaultArgPromote(type::complexFloat()).equivalentTo(type::complexFloat()));
    EXPECT_TRUE(type::defaultArgPromote(type::complexDouble()).equivalentTo(type::complexDouble()));
}

TEST(TypeQuery, enumUnderlyingTypeSelectsByRange) {
    EXPECT_TRUE(type::enumUnderlyingType(0, 1).equivalentTo(type::signedInteger()));
    EXPECT_TRUE(type::enumUnderlyingType(-1, 1).equivalentTo(type::signedInteger()));
    EXPECT_TRUE(type::enumUnderlyingType(0, 0x80000000L).equivalentTo(type::unsignedInteger()));
    EXPECT_TRUE(type::enumUnderlyingType(0x100000000L, 0x100000000L).equivalentTo(type::signedLong()));
    EXPECT_TRUE(type::enumUnderlyingType(0, 0x100000000L).equivalentTo(type::signedLong()));
    // Degenerate range: single constant uses the same policy.
    EXPECT_TRUE(type::enumUnderlyingType(42, 42).equivalentTo(type::signedInteger()));
    EXPECT_TRUE(type::enumUnderlyingType(0x80000000L, 0x80000000L).equivalentTo(type::unsignedInteger()));
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

} // namespace
