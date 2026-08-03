#ifndef UTIL_PRODUCTAPPROX_H_
#define UTIL_PRODUCTAPPROX_H_

// Documented product approximations for compiling git-shaped C.
// Contract tests: test/src/functionalTest/ProductContractsTest.cpp
// Prefer calling these named helpers over inline magic constants / special-case
// branches in SA or codegen visitors.
//
// Offsetof (&((T*)0)->m) is folded in SA using FieldPlan + null constant base
// (not a util→AST template).

namespace product_approx {

// __builtin_constant_p(x) always folds to 0 so both branches stay parseable
// (git-shaped constant-expression checks). See ProductContracts.constantPAlwaysZero.
inline long foldConstantP() { return 0; }

// __builtin_types_compatible_p always 0 (ARRAY_SIZE / BARF_UNLESS_AN_ARRAY path).
// See ProductContracts.typesCompatiblePAlwaysZero.
// AST-build path (CSNB_Builtins) emits ConstantExpression with this value.
inline long foldTypesCompatibleP() { return 0; }

// BUILD_ASSERT_OR_ZERO(cond) expands to (sizeof(char [1 - 2*!(cond)]) - 1).
// When cond is false the bound is -1. Real GCC rejects that; the product contract
// clamps the array length to 1 so sizeof(char[1]) - 1 == 0 and MOVE_ARRAY keeps
// element size sizeof(T). See ProductContracts.buildAssertOrZeroNegativeSizeofContributesZero.
// Only apply via named helper at known BUILD_ASSERT sites (CSNB_Types).
inline long clampNegativeArrayBoundForBuildAssert() { return 1; }

} // namespace product_approx

#endif // UTIL_PRODUCTAPPROX_H_
