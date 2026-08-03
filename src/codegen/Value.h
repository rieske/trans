#ifndef VALUE_H_
#define VALUE_H_

#include "codegen/Register.h"
#include "codegen/Sse.h"
#include "types/SysVClass.h"

#include <stdexcept>
#include <string>

namespace codegen {

enum class ValueKind {
    INTEGRAL, FLOATING, COMPLEX
};

// Register-allocation entity for temps and non-global named objects.
// Object homes are Address entries in StackMachine (global Values are resolve-only).
class Value {
public:
    // isSigned: Truncate and defaultClassification only. Loads use Classification.gprExtend.
    // Default true matches historical int/long/char treatment (EOF / negative ints).
    // Kind+size become the Classification when the C type is unknown (tests / temps).
    // FLOATING size > 8 is x87 (long double), not MEMORY/SSE.
    Value(std::string name, int index, ValueKind kind, int sizeInBytes, bool isSigned = true);
    Value(std::string name, int index, ValueKind kind, int sizeInBytes, bool isSigned,
            type::sysv::Classification classification);
    ~Value() = default;

    std::string getName() const;

    void assignRegister(Register* reg);
    void removeRegister(Register* reg);
    Register& getAssignedRegister() const;
    // True when no register holds this Value (memory / not yet loaded).
    bool isStored() const;

    int getIndex() const;
    ValueKind getValueKind() const;
    int getSizeInBytes() const;
    bool isSigned() const;
    type::sysv::Classification getClassification() const;

    // Last body-quad index that mentions this value, or -1 if live for the whole
    // procedure (named locals). Used so dead expression temps are discarded from
    // registers without writing a stack slot that may already be reused.
    void setLastUseOrdinal(int ordinal);
    int getLastUseOrdinal() const;

private:
    std::string name;
    int index;
    ValueKind valueKind_;
    int sizeInBytes;
    bool signedIntegral;
    type::sysv::Classification classification {};
    int lastUseOrdinal { -1 };

    Register* assignedRegister { nullptr };
};

inline bool isSseFloat32(const Value& v) {
    return v.getValueKind() == ValueKind::FLOATING && v.getSizeInBytes() == 4;
}

inline bool isSseFloat64(const Value& v) {
    return v.getValueKind() == ValueKind::FLOATING && v.getSizeInBytes() == 8;
}

inline bool isX87Float(const Value& v) {
    return v.getValueKind() == ValueKind::FLOATING && v.getSizeInBytes() == 16;
}

inline bool usesFpPath(const Value& v) {
    return v.getValueKind() == ValueKind::FLOATING || v.getValueKind() == ValueKind::COMPLEX;
}

// SSE only: 4 -> F32, 8 -> F64. x87 and non-floats throw.
inline SseWidth sseWidth(const Value& v) {
    if (isSseFloat32(v)) {
        return SseWidth::F32;
    }
    if (isSseFloat64(v)) {
        return SseWidth::F64;
    }
    throw std::runtime_error { "sseWidth requires an SSE float" };
}

} // namespace codegen

#endif // VALUE_H_
