#ifndef VALUE_H_
#define VALUE_H_

#include "codegen/Register.h"
#include "types/SysVClass.h"

namespace codegen {

enum class Type {
    INTEGRAL, FLOATING, COMPLEX
};

// Register-allocation entity for temps and non-global named objects.
// Object homes are Address entries in StackMachine (global Values are resolve-only).
class Value {
public:
    // Kind+size become the Classification (tests / Values with no C type).
    Value(int id, int index, Type type, int sizeInBytes);
    Value(int id, int index, Type type, int sizeInBytes,
            type::sysv::Classification classification);
    ~Value() = default;

    Value withIndex(int index) const;

    int id() const;

    void assignRegister(Register* reg);
    void removeRegister(Register* reg);
    Register& getAssignedRegister() const;
    // True when no register holds this Value (memory / not yet loaded).
    bool isStored() const;

    int getIndex() const;
    Type getType() const;
    int getSizeInBytes() const;
    type::sysv::Classification getClassification() const;

    void markExpressionTemp();
    bool isExpressionTemp() const;
    void setLastUseOrdinal(int ordinal);
    int getLastUseOrdinal() const;

private:
    int id_ { -1 };
    int index;
    Type type;
    int sizeInBytes;
    type::sysv::Classification classification {};

    Register* assignedRegister { nullptr };
    bool expressionTemp_ { false };
    int lastUseOrdinal_ { -1 };
};

inline bool isSseFloat32(const Value& v) {
    return v.getType() == Type::FLOATING && v.getSizeInBytes() == 4;
}

inline bool isSseFloat64(const Value& v) {
    return v.getType() == Type::FLOATING && v.getSizeInBytes() == 8;
}

inline bool isX87Float(const Value& v) {
    return v.getType() == Type::FLOATING && v.getSizeInBytes() == 16;
}

} // namespace codegen

#endif // VALUE_H_
