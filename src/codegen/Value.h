#ifndef VALUE_H_
#define VALUE_H_

#include "codegen/Register.h"
#include <string>

namespace codegen {

enum class Type {
    INTEGRAL, FLOATING
};

class Value {
public:
    Value(std::string name, int index, Type type, int sizeInBytes, bool functionArgument = false, bool global = false);
    ~Value() = default;

    std::string getName() const;

    void assignRegister(Register* reg);
    void removeRegister(Register* reg);
    Register& getAssignedRegister() const;
    bool isStored() const;

    bool isFunctionArgument() const;
    bool isGlobal() const;
    int getIndex() const;
    Type getType() const;
    int getSizeInBytes() const;

private:
    std::string name;
    int index;
    Type type;
    int sizeInBytes;
    bool functionArgument;
    bool global;

    Register* assignedRegister { nullptr };
};

} // namespace codegen

#endif // VALUE_H_
