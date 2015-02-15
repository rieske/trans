#ifndef _REGISTER_H_
#define _REGISTER_H_

#include <string>

namespace code_generator {

class Value;

enum regEnum {
    EAX, EBX, ECX, EDX
};

class Register {
public:
    Register(regEnum w);

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
