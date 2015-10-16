#ifndef REGISTER_H_
#define REGISTER_H_

#include <string>

namespace codegen {

class Value;

class Register {
public:
    explicit Register(std::string name);
    ~Register() = default;
    Register(const Register&) = delete;
    Register(Register&&) = delete;

    Register& operator=(const Register&) = delete;
    Register& operator=(Register&&) = delete;

    std::string getName() const;

    bool containsUnstoredValue() const;
    Value* getValue() const;

    void assign(Value* value);
    void free();

private:
    std::string name;

    Value* valueHeld { nullptr };
};

} /* namespace code_generator */

#endif /* REGISTER_H_ */
