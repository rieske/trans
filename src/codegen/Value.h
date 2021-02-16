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
    Value(std::string name, int index, Type type, int sizeInBytes, bool functionArgument = false);
    ~Value() = default;
    //Value(const Value&) = delete;
    //Value(Value&&) = delete;

    //Value& operator=(const Value&) = delete;
    //Value& operator=(Value&&) = delete;

    std::string getName() const;

    void assignRegister(Register* reg);
    void removeRegister(Register* reg);
    Register& getAssignedRegister() const;
    bool isStored() const;

    bool isFunctionArgument() const;
    int getIndex() const;
    Type getType() const;
    int getSizeInBytes() const;

private:
    std::string name;
    int index;
    Type type;
    int sizeInBytes;
    bool functionArgument;

    Register* assignedRegister { nullptr };
};

} // namespace codegen

#endif // VALUE_H_
