#ifndef SYMBOLS_ADDRESS_PLAN_H_
#define SYMBOLS_ADDRESS_PLAN_H_

// SA→CG address plans. symbols does not depend on ast:
// children/keys are NodeRef / ExpressionRef.

#include <string>
#include <variant>

#include "NodeRef.h"

namespace symbols {

// SA-owned base story for Field/Index IR (threaded through StackMachine).
// LeaObject:    LEA from object/array symbol (dot on plain object, array id).
// PointerValue: base holds a pointer/address value (arrow, p[i], nested via name).
enum class AddressBaseMode {
    LeaObject,
    PointerValue,
};

inline bool addressBaseUsesLea(AddressBaseMode mode) {
    return mode == AddressBaseMode::LeaObject;
}

inline bool addressBaseIsPointerValue(AddressBaseMode mode) {
    return mode == AddressBaseMode::PointerValue;
}

// SA resolves mode + symbol name together.
struct AddressBaseResolved {
    AddressBaseMode mode { AddressBaseMode::LeaObject };
    std::string name {};
};

struct FieldPlan {
    ExpressionRef baseExpr;
    int fieldOffsetBytes { 0 };
    AddressBaseResolved base {};
};

struct IndexPlan {
    ExpressionRef baseExpr;
    ExpressionRef indexExpr;
    int elementSize { 8 };
    AddressBaseResolved base {};
};

// Function designator: LEA of the function label into a pointer temp.
// Sole channel for designator name (no parallel IdentifierExpression string).
struct FunctionDesignatorPlan {
    std::string functionName {};
};

struct LvaluePlan {};
struct ResultAddressOfPlan {};

using AddressPlan = std::variant<
        FieldPlan,
        IndexPlan,
        FunctionDesignatorPlan,
        LvaluePlan,
        ResultAddressOfPlan>;

} // namespace symbols

#endif // SYMBOLS_ADDRESS_PLAN_H_
