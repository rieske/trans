#ifndef VALUE_H_
#define VALUE_H_

#include <string>

namespace codegen {

class Register;

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

} /* namespace code_generator */

#endif /* VALUE_H_ */
