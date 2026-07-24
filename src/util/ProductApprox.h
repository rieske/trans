#ifndef UTIL_PRODUCTAPPROX_H_
#define UTIL_PRODUCTAPPROX_H_

// Documented product approximations for compiling git-shaped C.
// Contract tests: test/src/functionalTest/ProductContractsTest.cpp
// Prefer calling these named helpers over inline magic constants / special-case
// branches in SA or codegen visitors.

#include <optional>
#include <string>

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
inline long clampNegativeArrayBoundForBuildAssert() { return 1; }

// Offsetof pattern: &((T*)0)->member folds to the field byte offset (git headers
// via __builtin_offsetof rewrite). Returns the offset when the operand matches.
// See ProductContracts / SizeofTest offsetof-style folds.
template <typename MemberAccess>
inline std::optional<int> foldOffsetofArrowFromNull(const MemberAccess* member) {
    if (!member || !member->isArrow()) {
        return std::nullopt;
    }
    long baseVal = 1;
    if (!member->getBase()->evaluateConstant(baseVal) || baseVal != 0) {
        return std::nullopt;
    }
    return member->getMemberOffset();
}

} // namespace product_approx

#endif // UTIL_PRODUCTAPPROX_H_
