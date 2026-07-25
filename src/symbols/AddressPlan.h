#ifndef SYMBOLS_ADDRESS_PLAN_H_
#define SYMBOLS_ADDRESS_PLAN_H_

#include <cstddef>
#include <functional>
#include <string>
#include <variant>

// SA→CG address plans (finish-for-git seam). symbols does not depend on ast:
// expression children are ExpressionRef; cast only in codegen if needed.

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

class ExpressionRef {
public:
    ExpressionRef() = default;
    template <typename T>
    ExpressionRef(const T* expr) : ptr_ { static_cast<const void*>(expr) } {}

    explicit operator bool() const { return ptr_ != nullptr; }

    template <typename T>
    T* as() const {
        return static_cast<T*>(const_cast<void*>(ptr_));
    }

private:
    const void* ptr_ { nullptr };
};

struct FieldPlan {
    ExpressionRef baseExpr;
    int fieldOffsetBytes { 0 };
    bool baseIsPointer { false };
    // Temp name for the computed field address (SA-allocated).
    std::string addressTempName;
};

struct IndexPlan {
    ExpressionRef baseExpr;
    ExpressionRef indexExpr;
    int elementSize { 8 };
    bool baseIsArray { false };
    std::string addressTempName;
};


using AddressPlan = std::variant<FieldPlan, IndexPlan>;

template <typename T>
inline const T* get_if(const AddressPlan* plan) {
    return plan ? std::get_if<T>(plan) : nullptr;
}

// SA→CG call shape (Normal only for now; host extends for builtins).
struct CallPlan {
    enum class Kind { Normal };
    Kind kind { Kind::Normal };
    bool indirect { false };
    // Direct: function label. Indirect: value holding the callee address.
    std::string calleeName;
};

// Brace / structure field init stores (string-named temps).
struct StructFieldInit {
    int offsetBytes { 0 };
    std::string addressName;
    std::string sourceName;
    bool zeroInitialize { false };
};

} // namespace symbols

#endif // SYMBOLS_ADDRESS_PLAN_H_
