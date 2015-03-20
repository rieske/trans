#ifndef _REGISTER_H_DEPRECATED
#define _REGISTER_H_DEPRECATED

#include <string>

namespace code_generator {

class Value;

enum regEnum {
    EAX, EBX, ECX, EDX
};

class Register_deprecated {
public:
    Register_deprecated(regEnum w);

    std::string getName() const;
    bool isFree() const;
    void setValue(Value* val);

    std::string free();

private:
    regEnum which;
    std::string name;
    Value *value;
};

}

#endif // _REGISTER_H_
