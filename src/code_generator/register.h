#ifndef _REGISTER_H_
#define _REGISTER_H_

#include <string>

namespace code_generator {
class ValueEntry;
} /* namespace code_generator */

namespace code_generator {

enum regEnum {
    EAX, EBX, ECX, EDX
};

class Register {
public:
    Register(regEnum w);

    std::string getName() const;
    bool isFree() const;
    void setValue(ValueEntry* val);

    std::string free();

private:
    regEnum which;
    std::string name;
    ValueEntry *value;
};

}

#endif // _REGISTER_H_
