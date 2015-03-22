#ifndef REGISTER_H_
#define REGISTER_H_

#include <string>

namespace codegen {

class Value;

class Register {
public:
    explicit Register(std::string name);
    Register(const Register&) = delete;
    Register(Register&&) = default;
    virtual ~Register() = default;

    Register& operator=(const Register&) = delete;
    Register& operator=(Register&&) = default;

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
