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
    Value(std::string name, int index, Type type, int sizeInBytes);
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

private:
    std::string name;
    int index;
    Type type;
    int sizeInBytes;

    Register* assignedRegister { nullptr };
};

} // namespace codegen

#endif // VALUE_H_
