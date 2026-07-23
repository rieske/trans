#ifndef VALUE_H_
#define VALUE_H_

#include "codegen/Register.h"
#include <string>

namespace codegen {

enum class Type {
    INTEGRAL, FLOATING
};

// Register-allocation entity for temps and non-global named objects.
// Object homes are Address entries in StackMachine (global Values are resolve-only).
class Value {
public:
    // isSigned: when sizeInBytes < 8, emitLoad zero- or sign-extends from memory.
    // Default true matches historical int/long/char treatment (EOF / negative ints).
    Value(std::string name, int index, Type type, int sizeInBytes, bool isSigned = true);
    ~Value() = default;

    std::string getName() const;

    void assignRegister(Register* reg);
    void removeRegister(Register* reg);
    Register& getAssignedRegister() const;
    // True when no register holds this Value (memory / not yet loaded).
    bool isStored() const;

    int getIndex() const;
    Type getType() const;
    int getSizeInBytes() const;
    bool isSigned() const;

    // Last body-quad index that mentions this value, or -1 if live for the whole
    // procedure (named locals). Used so dead expression temps are discarded from
    // registers without writing a stack slot that may already be reused.
    void setLastUseOrdinal(int ordinal);
    int getLastUseOrdinal() const;

private:
    std::string name;
    int index;
    Type type;
    int sizeInBytes;
    bool signedIntegral;
    int lastUseOrdinal { -1 };

    Register* assignedRegister { nullptr };
};

} // namespace codegen

#endif // VALUE_H_
