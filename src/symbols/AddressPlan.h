#ifndef SYMBOLS_ADDRESS_PLAN_H_
#define SYMBOLS_ADDRESS_PLAN_H_

#include <cstddef>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

#include "types/Type.h"

// SA→CG address plans (finish-for-git seam). symbols does not depend on ast.

namespace symbols {

class NodeRef {
public:
    NodeRef() = default;
    template <typename T>
    NodeRef(const T* node) : ptr_ { static_cast<const void*>(node) } {}

    explicit operator bool() const { return ptr_ != nullptr; }
    const void* get() const { return ptr_; }
    bool operator==(NodeRef o) const { return ptr_ == o.ptr_; }

private:
    const void* ptr_ { nullptr };
};

struct NodeRefHash {
    std::size_t operator()(NodeRef n) const noexcept {
        return std::hash<const void*> {}(n.get());
    }
};

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
    AddressBaseMode baseMode { AddressBaseMode::LeaObject };
    BinaryOperand baseOperand { BinaryOperand::Left };
};


// Address temp is the designator Result symbol on the store (not duplicated here).
// functionName is the linker label for a direct call; absent means call through the Result pointer.
struct FunctionDesignatorPlan {
    std::optional<std::string> functionName;
};

using AddressPlan = std::variant<FieldPlan, IndexPlan, FunctionDesignatorPlan>;

// SA→CG call shape — closed variant.
// Direct: calleeName is a function label. Indirect: calleeName is a value holding the address.
// Va* arms are compiler builtins, not libc calls.
struct DirectCallPlan {
    std::string calleeName;
};

struct IndirectCallPlan {
    std::string calleeName;
};

struct VaStartPlan {};
struct VaArgPlan {};
struct VaEndPlan {};
struct VaCopyPlan {};

using CallPlan = std::variant<DirectCallPlan, IndirectCallPlan, VaStartPlan, VaArgPlan, VaEndPlan, VaCopyPlan>;

template <typename T, typename Variant>
inline const T* get_if(const Variant* plan) {
    return plan ? std::get_if<T>(plan) : nullptr;
}

// Bit-field metadata if `plan` is a FieldPlan for a bit-field member; else null.
inline const type::BitField* bitFieldOf(const AddressPlan* plan) {
    const auto* field = get_if<FieldPlan>(plan);
    if (!field || !field->isBitField()) {
        return nullptr;
    }
    return &*field->bitField;
}

inline bool isIndirectCall(const CallPlan& plan) {
    return std::holds_alternative<IndirectCallPlan>(plan);
}

inline const std::string& callCalleeName(const CallPlan& plan) {
    return std::visit([](const auto& arm) -> const std::string& {
        using T = std::decay_t<decltype(arm)>;
        if constexpr (std::is_same_v<T, DirectCallPlan> || std::is_same_v<T, IndirectCallPlan>) {
            return arm.calleeName;
        } else {
            throw std::logic_error { "callCalleeName on non-call CallPlan" };
        }
    }, plan);
}

// Brace / structure field init stores (string-named temps).
struct StructFieldInit {
    int offsetBytes { 0 };
    std::string addressName;
    std::string sourceName;
    bool zeroInitialize { false };
    std::optional<std::string> immediate;
    std::optional<type::BitField> bitField;
    type::Type type { type::voidType() };

    bool isBitField() const { return bitField.has_value(); }
};

} // namespace symbols

#endif // SYMBOLS_ADDRESS_PLAN_H_
