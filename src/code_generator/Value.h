#ifndef VALUE_H_
#define VALUE_H_

#include <string>
#include <vector>

namespace code_generator {

enum class Type {
    INTEGRAL, FLOATING
};

class Value {
public:
    Value(std::string name, std::size_t index, Type type, bool functionArgument = false);

    std::string getName() const;

    void assignRegister(std::string reg);
    void removeReg(std::string reg);
    std::string getAssignedRegisterName() const;
    bool isStored() const;

    bool isFunctionArgument() const;
    std::size_t getIndex() const;
    Type getType() const;

private:
    std::string name;
    std::size_t index;
    Type type;
    bool functionArgument;

    std::string assignedRegisterName {};
};

} /* namespace code_generator */

#endif /* VALUE_H_ */
