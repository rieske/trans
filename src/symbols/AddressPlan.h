#ifndef SYMBOLS_ADDRESS_PLAN_H_
#define SYMBOLS_ADDRESS_PLAN_H_

// SA->CG address plans. symbols does not depend on ast.

#include <optional>
#include <string>
#include <variant>

#include "NodeRef.h"
#include "types/Type.h"

namespace symbols {

// LeaObject:    LEA from object home (plain struct, array id).
// PointerValue: base is a pointer value (arrow, p[i], struct [] / . lvalue).
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

struct FieldPlan {
    int fieldOffsetBytes { 0 };
    std::optional<type::BitField> bitField;

    bool isBitField() const { return bitField.has_value(); }
};

enum class BinaryOperand { Left, Right };

inline BinaryOperand otherBinaryOperand(BinaryOperand operand) {
    return operand == BinaryOperand::Left ? BinaryOperand::Right : BinaryOperand::Left;
}

template<typename T>
T& pickBinaryOperand(T& left, T& right, BinaryOperand operand) {
    return operand == BinaryOperand::Left ? left : right;
}

struct IndexPlan {
    int elementSize { 8 };
    type::Type elementType { type::voidType() };
    AddressBaseMode baseMode { AddressBaseMode::LeaObject };
    BinaryOperand baseOperand { BinaryOperand::Left };
};

// Function designator: LEA of the function label into a pointer temp.
// Sole channel for designator name (no parallel IdentifierExpression string).
// functionName is the linker label for a direct call; absent means call through
// the Result pointer.
struct FunctionDesignatorPlan {
    std::optional<std::string> functionName;
};

// Feature SA/CG extra plans (master AddressPlan is Field/Index/FunctionDesignator only).
struct LvaluePlan {};
struct ResultAddressOfPlan {};
struct ArrayDecayPlan {
    std::string objectName {};
};

using AddressPlan = std::variant<
        FieldPlan,
        IndexPlan,
        FunctionDesignatorPlan,
        LvaluePlan,
        ResultAddressOfPlan,
        ArrayDecayPlan>;

// Bit-field metadata if plan is a FieldPlan for a bit-field member; else null.
inline const type::BitField* bitFieldOf(const AddressPlan* plan) {
    const auto* field = get_if<FieldPlan>(plan);
    if (!field || !field->isBitField()) {
        return nullptr;
    }
    return &*field->bitField;
}

} // namespace symbols

#endif // SYMBOLS_ADDRESS_PLAN_H_
