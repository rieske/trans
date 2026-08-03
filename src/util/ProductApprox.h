#ifndef UTIL_PRODUCTAPPROX_H_
#define UTIL_PRODUCTAPPROX_H_

// Documented product approximations for compiling git-shaped C.
// Contract tests: test/src/functionalTest/ProductContractsTest.cpp
// Prefer calling these named helpers over inline magic constants / special-case
// branches in SA or codegen visitors.
//
// Offsetof (&((T*)0)->m) is folded in SA using FieldPlan + null constant base
// (not a util→AST template). __builtin_offsetof is a parse-time ConstantExpression.

namespace product_approx {

// BUILD_ASSERT_OR_ZERO(cond) expands to (sizeof(char [1 - 2*!(cond)]) - 1).
// When cond is false the bound is -1. Real GCC rejects that; the product contract
// clamps the array length to 1 so sizeof(char[1]) - 1 == 0 and MOVE_ARRAY keeps
// element size sizeof(T). See ProductContracts.buildAssertOrZeroNegativeSizeofContributesZero.
// Applied from ArrayDeclarator::getFundamentalType when a constant bound is < 0
// (sizeof(char[N]) type-name path used by BUILD_ASSERT_OR_ZERO).
inline long clampNegativeArrayBoundForBuildAssert() { return 1; }

} // namespace product_approx

#endif // UTIL_PRODUCTAPPROX_H_
